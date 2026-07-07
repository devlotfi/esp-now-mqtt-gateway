#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <uri/UriBraces.h>
#include <etl/string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "preferences/Grafana.h"
#include "preferences/Notifications.h"
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "Lookup.h"
#include "EspNow.h"

#define HTTP_DISPATCHER_QUEUE_LENGTH 10

esp_err_t http_dispatcher_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id)
  {
  case HTTP_EVENT_ON_CONNECTED:
    Serial.println("HTTP_EVENT_ON_CONNECTED");
    break;

  case HTTP_EVENT_HEADERS_SENT:
    Serial.println("HTTP_EVENT_HEADERS_SENT");
    break;

  case HTTP_EVENT_ON_DATA:
    Serial.print("Response Body: ");
    Serial.printf("%.*s\n", evt->data_len, (char *)evt->data);
    break;

  case HTTP_EVENT_ON_FINISH:
    Serial.println("HTTP_EVENT_ON_FINISH");
    break;

  case HTTP_EVENT_DISCONNECTED:
    Serial.println("HTTP_EVENT_DISCONNECTED");
    break;

  case HTTP_EVENT_ERROR:
    Serial.println("HTTP_EVENT_ERROR");
    break;

  default:
    break;
  }
  return ESP_OK;
}

enum RequestKind : uint8_t
{
  NOTIFICATION_TEST,
  NOTIFICATION_SEND,
  GRAFANA_METRIC
};

struct NotificationSendPayload
{
  char title[NOTIFICATION_TITLE_SIZE];
  char body[NOTIFICATION_BODY_SIZE];
};

struct GrafanaMetricPayload
{
  char body[GRAFANA_BODY_SIZE];
};

struct HttpDispatcherQueueItem
{
  RequestKind requestKind;
  union
  {
    NotificationSendPayload notificationSendPayload;
    GrafanaMetricPayload grafanaMetricPayload;
  } payload;
};

static QueueHandle_t httpDispatcherQueue = nullptr;
static esp_http_client_handle_t httpDispatcherClient = nullptr;

static bool sendRequest(HttpDispatcherQueueItem *httpDispatcherQueueItem)
{
  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000;

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  esp_http_client_delete_header(httpDispatcherClient, "x-api-key");
  esp_http_client_delete_header(httpDispatcherClient, "Content-Type");
  esp_http_client_delete_header(httpDispatcherClient, "Authorization");
  esp_http_client_set_post_field(httpDispatcherClient, NULL, 0);

  while (attempt < MAX_RETRIES)
  {
    switch (httpDispatcherQueueItem->requestKind)
    {
    case RequestKind::NOTIFICATION_TEST:
    {
      NotificationsData *notificationsData = loadNotificationsData();
      if (!notificationsData->isSet)
        return true;

      etl::string<NOTIFICATIONS_API_URL_SIZE> url;
      url = notificationsData->apiUrl;
      if (!url.empty() && url.back() == '/')
        url += "api/subscriptions/test";
      else
        url += "/api/subscriptions/test";

      esp_http_client_set_url(httpDispatcherClient, url.c_str());
      esp_http_client_set_method(httpDispatcherClient, HTTP_METHOD_POST);
      esp_http_client_set_header(httpDispatcherClient, "x-api-key", notificationsData->apiSecret);
      break;
    }
    case RequestKind::NOTIFICATION_SEND:
    {
      NotificationsData *notificationsData = loadNotificationsData();
      if (!notificationsData->isSet)
        return true;

      etl::string<NOTIFICATIONS_API_URL_SIZE> url;
      url = notificationsData->apiUrl;
      if (!url.empty() && url.back() == '/')
        url += "api/subscriptions/send";
      else
        url += "/api/subscriptions/send";

      char post_data[2048] = {0};
      ArduinoJson::JsonDocument doc;
      doc["title"] = httpDispatcherQueueItem->payload.notificationSendPayload.title;
      doc["body"] = httpDispatcherQueueItem->payload.notificationSendPayload.body;
      ArduinoJson::serializeJson(doc, post_data, sizeof(post_data));

      esp_http_client_set_url(httpDispatcherClient, url.c_str());
      esp_http_client_set_method(httpDispatcherClient, HTTP_METHOD_POST);
      esp_http_client_set_header(httpDispatcherClient, "x-api-key", notificationsData->apiSecret);
      esp_http_client_set_header(httpDispatcherClient, "Content-Type", "application/json");
      esp_http_client_set_post_field(httpDispatcherClient, post_data, strlen(post_data));
      break;
    }
    case RequestKind::GRAFANA_METRIC:
    {
      GrafanaData *grafanaData = loadGrafanaData();
      if (!grafanaData->isSet)
        return true;

      etl::string<2048> authorization = "Bearer ";
      authorization += grafanaData->instanceId;
      authorization += ":";
      authorization += grafanaData->apiKey;

      esp_http_client_set_url(httpDispatcherClient, grafanaData->url);
      esp_http_client_set_method(httpDispatcherClient, HTTP_METHOD_POST);
      esp_http_client_set_header(httpDispatcherClient, "Authorization", authorization.c_str());
      esp_http_client_set_header(httpDispatcherClient, "Content-Type", "application/json");
      esp_http_client_set_post_field(
          httpDispatcherClient,
          httpDispatcherQueueItem->payload.grafanaMetricPayload.body,
          strlen(httpDispatcherQueueItem->payload.grafanaMetricPayload.body));
      break;
    }
    default:
      break;
    }

    err = esp_http_client_perform(httpDispatcherClient);

    if (err != ESP_OK)
    {
      Serial.printf("HTTP transport error (attempt %d): %s\n",
                    attempt + 1,
                    esp_err_to_name(err));

      esp_http_client_close(httpDispatcherClient);
    }
    else
    {
      int status = esp_http_client_get_status_code(httpDispatcherClient);
      Serial.printf("HTTP Status = %d\n", status);

      if (status >= 200 && status < 300)
      {
        // Success
        return true;
      }

      if (status == 429 || status >= 500)
      {
        // Retryable server-side errors
        Serial.printf("Retryable HTTP error %d (attempt %d)\n",
                      status,
                      attempt + 1);

        esp_http_client_close(httpDispatcherClient);
      }
      else
      {
        // 4xx errors are usually permanent
        Serial.printf("Non-retryable HTTP error %d\n", status);
        return false;
      }
    }

    attempt++;

    if (attempt < MAX_RETRIES)
    {
      vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
  }

  Serial.println("HTTP request ultimately failed after retries");
  return false;
}

static void httpDispatcherWorkerTask(void *pvParameters)
{
  // Created once, lives for the lifetime of the device.
  esp_http_client_config_t config = {
      .url = "https://localhost", // placeholder; overwritten per-request via set_url
      .method = HTTP_METHOD_POST,
      .event_handler = http_dispatcher_event_handler,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .keep_alive_enable = true,
  };

  httpDispatcherClient = esp_http_client_init(&config);
  if (!httpDispatcherClient)
  {
    Serial.println("ERROR: failed to init http dispatcher client, worker exiting");
    vTaskDelete(nullptr);
    return;
  }

  HttpDispatcherQueueItem *httpDispatcherQueueItem;

  for (;;)
  {
    // Blocks here indefinitely - no CPU/heap churn between requests.
    if (xQueueReceive(httpDispatcherQueue, &httpDispatcherQueueItem, portMAX_DELAY) == pdTRUE)
    {
      sendRequest(httpDispatcherQueueItem);
      heap_caps_free(httpDispatcherQueueItem);
    }
  }
}

// Call once from setup().
void initHttpDispatcherWorker()
{
  if (httpDispatcherQueue != nullptr)
  {
    return; // already initialized
  }

  httpDispatcherQueue = xQueueCreateWithCaps(
      HTTP_DISPATCHER_QUEUE_LENGTH,
      sizeof(HttpDispatcherQueueItem *),
      MALLOC_CAP_SPIRAM);
  if (!httpDispatcherQueue)
  {
    Serial.println("ERROR: failed to create http dispatcher queue");
    return;
  }

  xTaskCreatePinnedToCoreWithCaps(
      httpDispatcherWorkerTask,
      "http_dispatcher_worker",
      16384,
      nullptr,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}

void sendTestNotification()
{
  if (httpDispatcherQueue == nullptr)
  {
    Serial.println("ERROR: http dispatcher worker not initialized");
    return;
  }

  HttpDispatcherQueueItem *httpDispatcherQueueItem = (HttpDispatcherQueueItem *)heap_caps_malloc(sizeof(HttpDispatcherQueueItem), MALLOC_CAP_SPIRAM);
  httpDispatcherQueueItem->requestKind = RequestKind::NOTIFICATION_TEST;

  if (xQueueSend(httpDispatcherQueue, &httpDispatcherQueueItem, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(httpDispatcherQueueItem);
    Serial.println("ERROR: notifications queue full, dropping notification");
  }
}

void sendNotification(const char *title, const char *body)
{
  if (httpDispatcherQueue == nullptr)
  {
    Serial.println("ERROR: http dispatcher worker not initialized");
    return;
  }

  HttpDispatcherQueueItem *httpDispatcherQueueItem = (HttpDispatcherQueueItem *)heap_caps_malloc(sizeof(HttpDispatcherQueueItem), MALLOC_CAP_SPIRAM);
  httpDispatcherQueueItem->requestKind = RequestKind::NOTIFICATION_SEND;
  strlcpy(httpDispatcherQueueItem->payload.notificationSendPayload.title, title, sizeof(httpDispatcherQueueItem->payload.notificationSendPayload.title));
  strlcpy(httpDispatcherQueueItem->payload.notificationSendPayload.body, body, sizeof(httpDispatcherQueueItem->payload.notificationSendPayload.body));

  if (xQueueSend(httpDispatcherQueue, &httpDispatcherQueueItem, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(httpDispatcherQueueItem);
    Serial.println("ERROR: notifications queue full, dropping notification");
  }
}

void saveGrafanaMetric(const char *body)
{
  if (httpDispatcherQueue == nullptr)
  {
    Serial.println("ERROR: http dispatcher worker not initialized");
    return;
  }

  GrafanaData *grafanaData = loadGrafanaData();
  HttpDispatcherQueueItem *httpDispatcherQueueItem = (HttpDispatcherQueueItem *)heap_caps_malloc(sizeof(HttpDispatcherQueueItem), MALLOC_CAP_SPIRAM);
  httpDispatcherQueueItem->requestKind = RequestKind::GRAFANA_METRIC;
  strlcpy(httpDispatcherQueueItem->payload.grafanaMetricPayload.body, body, sizeof(httpDispatcherQueueItem->payload.grafanaMetricPayload.body));

  if (xQueueSend(httpDispatcherQueue, &httpDispatcherQueueItem, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(httpDispatcherQueueItem);
    Serial.println("ERROR: grafana queue full, dropping metric");
  }
}