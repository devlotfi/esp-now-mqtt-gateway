#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <uri/UriBraces.h>
#include <etl/string.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "preferences/Grafana.h"
#include "Lookup.h"
#include "EspNow.h"

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

struct GrafanaTaskArgs
{
  char url[GRAFANA_URL_SIZE];
  char instanceId[GRAFANA_INSTANCE_ID_SIZE];
  char apiKey[GRAFANA_API_KEY_SIZE];
  char body[GRAFANA_BODY_SIZE];
};

static void grafanaTask(void *pvParameters)
{
  GrafanaTaskArgs *args = (GrafanaTaskArgs *)pvParameters;

  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000;

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  while (attempt < MAX_RETRIES)
  {
    esp_http_client_config_t config = {
        .url = args->url,
        .method = HTTP_METHOD_POST,
        .event_handler = grafana_http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set headers
    etl::string<2048> authorization = "Bearer ";
    authorization += args->instanceId;
    authorization += ":";
    authorization += args->apiKey;
    esp_http_client_set_header(client, "Authorization", authorization.c_str());
    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Attach the JSON body
    esp_http_client_set_post_field(client, args->body, strlen(args->body));

    // Log request
    Serial.println("\n========== HTTP REQUEST ==========");
    Serial.printf("Attempt : %d\n", attempt + 1);
    Serial.printf("URL     : %s\n", args->url);
    Serial.println("Headers:");
    Serial.printf("  Authorization: %s\n", authorization.c_str());
    Serial.println("  Content-Type: application/json");
    Serial.println("Body:");
    Serial.println(args->body);
    Serial.println("==================================");

    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
      int status = esp_http_client_get_status_code(client);
      Serial.printf("HTTP Status = %d\n", status);

      esp_http_client_cleanup(client);
      break; // Success, exit retry loop
    }
    else
    {
      Serial.printf("HTTP request failed (attempt %d): %s\n",
                    attempt + 1,
                    esp_err_to_name(err));

      esp_http_client_cleanup(client);
      attempt++;

      if (attempt < MAX_RETRIES)
      {
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
      }
    }
  }

  if (err != ESP_OK)
  {
    Serial.println("HTTP request ultimately failed after retries");
  }

  free(args);
  vTaskDelete(nullptr);
}

void saveGrafanaMetric(GrafanaData *grafanaData, const char *body)
{
  GrafanaTaskArgs *args = (GrafanaTaskArgs *)malloc(sizeof(GrafanaTaskArgs));
  if (!args)
  {
    Serial.println("ERROR: OUT_OF_MEMORY");
    return;
  }

  // FIX: Pass standard sizeof() without "- 1" to strlcpy
  strlcpy(args->url, grafanaData->url, sizeof(args->url));
  strlcpy(args->instanceId, grafanaData->instanceId, sizeof(args->instanceId));
  strlcpy(args->apiKey, grafanaData->apiKey, sizeof(args->apiKey));
  strlcpy(args->body, body, sizeof(args->body));

  xTaskCreatePinnedToCoreWithCaps(
      grafanaTask,
      "save_grafana_metric_worker",
      8192,
      args,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}