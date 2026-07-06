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
#include "preferences/Grafana.h"
#include "Lookup.h"
#include "EspNow.h"

#define GRAFANA_QUEUE_LENGTH 10

// Corrected: Handlers process data during esp_http_client_perform()
esp_err_t grafana_http_event_handler(esp_http_client_event_t *evt)
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

struct GrafanaQueueItem
{
  char url[GRAFANA_URL_SIZE];
  char instanceId[GRAFANA_INSTANCE_ID_SIZE];
  char apiKey[GRAFANA_API_KEY_SIZE];
  char body[GRAFANA_BODY_SIZE];
};

static QueueHandle_t grafanaQueue = nullptr;
static esp_http_client_handle_t grafanaClient = nullptr;

static bool sendGrafanaRequest(GrafanaQueueItem *item)
{
  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000;

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  etl::string<2048> authorization = "Bearer ";
  authorization += item->instanceId;
  authorization += ":";
  authorization += item->apiKey;

  while (attempt < MAX_RETRIES)
  {
    // Reconfigure the persistent client for this request instead of
    // creating/destroying a new one every time.
    esp_http_client_set_url(grafanaClient, item->url);
    esp_http_client_set_method(grafanaClient, HTTP_METHOD_POST);
    esp_http_client_set_header(grafanaClient, "Authorization", authorization.c_str());
    esp_http_client_set_header(grafanaClient, "Content-Type", "application/json");
    esp_http_client_set_post_field(grafanaClient, item->body, strlen(item->body));

    Serial.println("\n========== HTTP REQUEST ==========");
    Serial.printf("Attempt : %d\n", attempt + 1);
    Serial.printf("URL     : %s\n", item->url);
    Serial.println("Headers:");
    Serial.printf("  Authorization: %s\n", authorization.c_str());
    Serial.println("  Content-Type: application/json");
    Serial.println("Body:");
    Serial.println(item->body);
    Serial.println("==================================");

    err = esp_http_client_perform(grafanaClient);

    if (err == ESP_OK)
    {
      int status = esp_http_client_get_status_code(grafanaClient);
      Serial.printf("HTTP Status = %d\n", status);
      return true;
    }
    else
    {
      Serial.printf("HTTP request failed (attempt %d): %s\n",
                    attempt + 1,
                    esp_err_to_name(err));

      // FIX: close (but don't cleanup) the connection after a failure so the
      // next attempt/request opens a fresh socket instead of reusing one
      // that's potentially in a bad state. The client handle itself persists.
      esp_http_client_close(grafanaClient);

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

static void grafanaWorkerTask(void *pvParameters)
{
  // Created once, lives for the lifetime of the device.
  esp_http_client_config_t config = {
      .url = "https://localhost", // placeholder; overwritten per-request via set_url
      .method = HTTP_METHOD_POST,
      .event_handler = grafana_http_event_handler,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .keep_alive_enable = true,
  };

  grafanaClient = esp_http_client_init(&config);
  if (!grafanaClient)
  {
    Serial.println("ERROR: failed to init grafana http client, worker exiting");
    vTaskDelete(nullptr);
    return;
  }

  GrafanaQueueItem *item;

  for (;;)
  {
    // Blocks here indefinitely - no CPU/heap churn between requests.
    if (xQueueReceive(grafanaQueue, &item, portMAX_DELAY) == pdTRUE)
    {
      sendGrafanaRequest(item);
      heap_caps_free(item);
    }
  }
}

// Call once from setup().
void initGrafanaWorker()
{
  if (grafanaQueue != nullptr)
  {
    return; // already initialized
  }

  grafanaQueue = xQueueCreateWithCaps(
      GRAFANA_QUEUE_LENGTH,
      sizeof(GrafanaQueueItem *),
      MALLOC_CAP_SPIRAM);
  if (!grafanaQueue)
  {
    Serial.println("ERROR: failed to create grafana queue");
    return;
  }

  xTaskCreatePinnedToCoreWithCaps(
      grafanaWorkerTask,
      "grafana_worker",
      16384,
      nullptr,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}

// Non-blocking: enqueues the metric and returns immediately.
void saveGrafanaMetric(GrafanaData *grafanaData, const char *body)
{
  if (grafanaQueue == nullptr)
  {
    Serial.println("ERROR: grafana worker not initialized - call initGrafanaWorker() in setup()");
    return;
  }

  GrafanaQueueItem *item = (GrafanaQueueItem *)heap_caps_malloc(sizeof(GrafanaQueueItem), MALLOC_CAP_SPIRAM);
  strlcpy(item->url, grafanaData->url, sizeof(item->url));
  strlcpy(item->instanceId, grafanaData->instanceId, sizeof(item->instanceId));
  strlcpy(item->apiKey, grafanaData->apiKey, sizeof(item->apiKey));
  strlcpy(item->body, body, sizeof(item->body));

  // Item is copied by value into the queue's internal storage - no malloc/free per call.
  if (xQueueSend(grafanaQueue, &item, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    heap_caps_free(item);
    Serial.println("ERROR: grafana queue full, dropping metric");
  }
}