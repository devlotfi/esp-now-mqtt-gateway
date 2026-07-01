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
#include "preferences/Notifications.h"
#include "Lookup.h"
#include "EspNow.h"

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

struct TestNotificationTaskArgs
{
  char url[NOTIFICATIONS_API_URL_SIZE];
  char apiSecret[NOTIFICATIONS_API_SECRET_SIZE];
};

static void testNotificationTask(void *pvParameters)
{
  TestNotificationTaskArgs *args = (TestNotificationTaskArgs *)pvParameters;

  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000; // 2s between retries

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  while (attempt < MAX_RETRIES)
  {
    esp_http_client_config_t config = {
        .url = args->url,
        .method = HTTP_METHOD_POST,
        .event_handler = notifications_http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "x-api-key", args->apiSecret);

    Serial.printf("HTTP attempt %d...\n", attempt + 1);

    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
      int status = esp_http_client_get_status_code(client);
      int content_length = esp_http_client_get_content_length(client);

      Serial.printf("HTTP Status = %d, content_length = %d\n",
                    status,
                    content_length);

      // FIX: Removed manual read loop here as data flows through the event handler
      esp_http_client_cleanup(client);
      break;
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

void sendTestNotification(NotificationsData *notificationsData)
{
  TestNotificationTaskArgs *args = (TestNotificationTaskArgs *)malloc(sizeof(TestNotificationTaskArgs));
  if (!args)
  {
    Serial.println("ERROR: OUT_OF_MEMORY");
    return;
  }

  // Build URL
  etl::string<NOTIFICATIONS_API_URL_SIZE> url;
  url = notificationsData->apiUrl;
  if (!url.empty() && url.back() == '/')
    url += "api/subscriptions/test";
  else
    url += "/api/subscriptions/test";

  // FIX: Passing standard sizeof() to strlcpy
  strlcpy(args->url, url.c_str(), sizeof(args->url));
  strlcpy(args->apiSecret, notificationsData->apiSecret, sizeof(args->apiSecret));

  xTaskCreatePinnedToCoreWithCaps(
      testNotificationTask,
      "test_notification_worker",
      8192,
      args,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}

struct NotificationTaskArgs
{
  char url[NOTIFICATIONS_API_URL_SIZE];
  char apiSecret[NOTIFICATIONS_API_SECRET_SIZE];
  char title[NOTIFICATION_TITLE_SIZE];
  char body[NOTIFICATION_BODY_SIZE];
};

static void notificationTask(void *pvParameters)
{
  NotificationTaskArgs *args = (NotificationTaskArgs *)pvParameters;

  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 2000;

  esp_err_t err = ESP_FAIL;
  int attempt = 0;

  char post_data[2048];
  ArduinoJson::JsonDocument doc;
  doc["title"] = args->title;
  doc["body"] = args->body;
  ArduinoJson::serializeJson(doc, post_data, 2048);

  while (attempt < MAX_RETRIES)
  {
    esp_http_client_config_t config = {
        .url = args->url,
        .method = HTTP_METHOD_POST,
        .event_handler = notifications_http_event_handler, // FIX: Registered the handler
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set headers
    esp_http_client_set_header(client, "x-api-key", args->apiSecret);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Attach the JSON body
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    Serial.printf("HTTP attempt %d...\n", attempt + 1);

    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
      int status = esp_http_client_get_status_code(client);
      Serial.printf("HTTP Status = %d\n", status);

      // FIX: Removed manual read loop here as data flows through the event handler
      esp_http_client_cleanup(client);
      break;
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

void sendNotification(NotificationsData *notificationsData, const char *title, const char *body)
{
  NotificationTaskArgs *args = (NotificationTaskArgs *)malloc(sizeof(NotificationTaskArgs));
  if (!args)
  {
    Serial.println("ERROR: OUT_OF_MEMORY");
    return;
  }

  // Build URL
  etl::string<NOTIFICATIONS_API_URL_SIZE> url;
  url = notificationsData->apiUrl;
  if (!url.empty() && url.back() == '/')
    url += "api/subscriptions/send";
  else
    url += "/api/subscriptions/send";

  // FIX: Passing standard sizeof() to strlcpy
  strlcpy(args->url, url.c_str(), sizeof(args->url));
  strlcpy(args->apiSecret, notificationsData->apiSecret, sizeof(args->apiSecret));
  strlcpy(args->title, title, sizeof(args->title));
  strlcpy(args->body, body, sizeof(args->body));

  xTaskCreatePinnedToCoreWithCaps(
      notificationTask,
      "send_notification_worker",
      8192,
      args,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_SPIRAM);
}