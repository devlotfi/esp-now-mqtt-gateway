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
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "preferences/Notifications.h"
#include "Lookup.h"
#include "EspNow.h"

#define NOTIFICATIONS_QUEUE_LENGTH 5

enum class NotificationKind : uint8_t
{
  TEST,
  SEND
};

esp_err_t notifications_http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id)
  {
  case HTTP_EVENT_ON_DATA:
    printf("%.*s", evt->data_len, (char *)evt->data);
    break;

  case HTTP_EVENT_ON_FINISH:
    printf("\n--- Request finished ---\n");
    break;

  case HTTP_EVENT_ERROR:
    printf("HTTP error\n");
    break;

  default:
    break;
  }
  return ESP_OK;
}

struct NotificationQueueItem
{
  NotificationKind kind;
  char url[NOTIFICATIONS_API_URL_SIZE];
  char apiSecret[NOTIFICATIONS_API_SECRET_SIZE];
  char title[NOTIFICATION_TITLE_SIZE];
  char body[NOTIFICATION_BODY_SIZE];
};

static QueueHandle_t notificationsQueue = nullptr;
static esp_http_client_handle_t notificationsClient = nullptr;

static bool sendNotificationRequest(NotificationQueueItem *item)
{
  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000;

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  // Build JSON body only for SEND; TEST posts an empty body.
  char post_data[2048] = {0};
  if (item->kind == NotificationKind::SEND)
  {
    ArduinoJson::JsonDocument doc;
    doc["title"] = item->title;
    doc["body"] = item->body;
    ArduinoJson::serializeJson(doc, post_data, sizeof(post_data));
  }

  while (attempt < MAX_RETRIES)
  {
    // Reconfigure the persistent client for this request instead of
    // creating/destroying a new one every time.
    esp_http_client_set_url(notificationsClient, item->url);
    esp_http_client_set_method(notificationsClient, HTTP_METHOD_POST);
    esp_http_client_set_header(notificationsClient, "x-api-key", item->apiSecret);

    if (item->kind == NotificationKind::SEND)
    {
      esp_http_client_set_header(notificationsClient, "Content-Type", "application/json");
      esp_http_client_set_post_field(notificationsClient, post_data, strlen(post_data));
    }
    else
    {
      // No body for test notification.
      esp_http_client_set_post_field(notificationsClient, nullptr, 0);
    }

    Serial.printf("HTTP attempt %d...\n", attempt + 1);

    err = esp_http_client_perform(notificationsClient);

    if (err == ESP_OK)
    {
      int status = esp_http_client_get_status_code(notificationsClient);
      int content_length = esp_http_client_get_content_length(notificationsClient);

      Serial.printf("HTTP Status = %d, content_length = %d\n", status, content_length);
      return true;
    }
    else
    {
      Serial.printf("HTTP request failed (attempt %d): %s\n",
                    attempt + 1,
                    esp_err_to_name(err));

      // FIX: close (not cleanup) so the next attempt opens a fresh socket
      // without destroying/reallocating the persistent client.
      esp_http_client_close(notificationsClient);

      attempt++;

      if (attempt < MAX_RETRIES)
      {
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
      }
    }
  }

  Serial.println("HTTP request ultimately failed after retries");
  return false;
}

static void notificationsWorkerTask(void *pvParameters)
{
  // Created once, lives for the lifetime of the device.
  esp_http_client_config_t config = {
      .url = "https://localhost", // placeholder; overwritten per-request via set_url
      .method = HTTP_METHOD_POST,
      .event_handler = notifications_http_event_handler,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .keep_alive_enable = true,
  };

  notificationsClient = esp_http_client_init(&config);
  if (!notificationsClient)
  {
    Serial.println("ERROR: failed to init notifications http client, worker exiting");
    vTaskDelete(nullptr);
    return;
  }

  NotificationQueueItem *item;

  for (;;)
  {
    // Blocks here indefinitely - no CPU/heap churn between requests.
    if (xQueueReceive(notificationsQueue, &item, portMAX_DELAY) == pdTRUE)
    {
      sendNotificationRequest(item);
      heap_caps_free(item);
    }
  }
}

// Call once from setup().
void initNotificationsWorker()
{
  if (notificationsQueue != nullptr)
  {
    return; // already initialized
  }

  notificationsQueue = xQueueCreateWithCaps(
      NOTIFICATIONS_QUEUE_LENGTH,
      sizeof(NotificationQueueItem *),
      MALLOC_CAP_SPIRAM);
  if (!notificationsQueue)
  {
    Serial.println("ERROR: failed to create notifications queue");
    return;
  }

  xTaskCreatePinnedToCoreWithCaps(
      notificationsWorkerTask,
      "notifications_worker",
      16384,
      nullptr,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}

static void buildTestUrl(NotificationsData *notificationsData, char *out, size_t outSize)
{
  etl::string<NOTIFICATIONS_API_URL_SIZE> url;
  url = notificationsData->apiUrl;
  if (!url.empty() && url.back() == '/')
    url += "api/subscriptions/test";
  else
    url += "/api/subscriptions/test";
  strlcpy(out, url.c_str(), outSize);
}

static void buildSendUrl(NotificationsData *notificationsData, char *out, size_t outSize)
{
  etl::string<NOTIFICATIONS_API_URL_SIZE> url;
  url = notificationsData->apiUrl;
  if (!url.empty() && url.back() == '/')
    url += "api/subscriptions/send";
  else
    url += "/api/subscriptions/send";
  strlcpy(out, url.c_str(), outSize);
}

// Non-blocking: enqueues the test notification and returns immediately.
void sendTestNotification(NotificationsData *notificationsData)
{
  heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
  if (notificationsQueue == nullptr)
  {
    Serial.println("ERROR: notifications worker not initialized - call initNotificationsWorker() in setup()");
    return;
  }

  NotificationQueueItem *item = (NotificationQueueItem *)heap_caps_malloc(sizeof(NotificationQueueItem), MALLOC_CAP_SPIRAM);
  item->kind = NotificationKind::TEST;
  buildTestUrl(notificationsData, item->url, sizeof(item->url));
  strlcpy(item->apiSecret, notificationsData->apiSecret, sizeof(item->apiSecret));

  if (xQueueSend(notificationsQueue, &item, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(item);
    Serial.println("ERROR: notifications queue full, dropping test notification");
  }
}

// Non-blocking: enqueues the notification and returns immediately.
void sendNotification(NotificationsData *notificationsData, const char *title, const char *body)
{
  if (notificationsQueue == nullptr)
  {
    Serial.println("ERROR: notifications worker not initialized - call initNotificationsWorker() in setup()");
    return;
  }

  NotificationQueueItem *item = (NotificationQueueItem *)heap_caps_malloc(sizeof(NotificationQueueItem), MALLOC_CAP_SPIRAM);
  item->kind = NotificationKind::SEND;
  buildSendUrl(notificationsData, item->url, sizeof(item->url));
  strlcpy(item->apiSecret, notificationsData->apiSecret, sizeof(item->apiSecret));
  strlcpy(item->title, title, sizeof(item->title));
  strlcpy(item->body, body, sizeof(item->body));

  if (xQueueSend(notificationsQueue, &item, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(item);
    Serial.println("ERROR: notifications queue full, dropping notification");
  }
}