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
#include "NvsData.h"
#include "Lookup.h"
#include "EspNow.h"

static StackType_t xStack[8192];
static StaticTask_t xTaskBuffer;

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id)
  {
  case HTTP_EVENT_ON_DATA:
    printf("Received data: %.*s\n", evt->data_len, (char *)evt->data);
    break;

  case HTTP_EVENT_ON_FINISH:
    printf("Request finished\n");
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
  char url[NOTIFICATIONS_API_URL_SIZE + 32];
  char apiSecret[NOTIFICATIONS_API_SECRET_SIZE];
};

static void testNotificationTask(void *pvParameters)
{
  Serial.printf("HTTP task: free heap = %lu, largest block = %lu\n",
                (unsigned long)ESP.getFreeHeap(),
                (unsigned long)ESP.getMaxAllocHeap());

  TestNotificationTaskArgs *args = (TestNotificationTaskArgs *)pvParameters;

  esp_http_client_config_t config = {
      .url = args->url,
      .method = HTTP_METHOD_POST,
      .buffer_size = 4096,
      .buffer_size_tx = 2048,
      .crt_bundle_attach = esp_crt_bundle_attach,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);

  // headers
  esp_http_client_set_header(client, "x-api-key", args->apiSecret);

  // perform request (blocking)
  esp_err_t err = esp_http_client_perform(client);

  if (err == ESP_OK)
  {
    int status = esp_http_client_get_status_code(client);
    int content_length = esp_http_client_get_content_length(client);

    Serial.printf("HTTP Status = %d, content_length = %d\n",
                  status,
                  content_length);

    char buffer[512];
    int read_len;

    while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1)) > 0)
    {
      buffer[read_len] = 0;
      Serial.print(buffer);
    }
  }
  else
  {
    Serial.printf("HTTP request failed: %s\n", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);

  free(args);
  vTaskDelete(nullptr);
}

class NotificationsController
{
public:
  static void configure(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["apiUrl"].is<const char *>() ||
        !json["apiSecret"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *apiUrl = json["apiUrl"].as<const char *>();
    const char *apiSecret = json["apiSecret"].as<const char *>();
    if (!isValidUrl(apiUrl))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    NotificationsData *notificationsData = loadNotificationsData();

    // saving in NVS
    notificationsData->isSet = true;
    strncpy(notificationsData->apiUrl, apiUrl, NOTIFICATIONS_API_URL_SIZE);
    notificationsData->apiUrl[NOTIFICATIONS_API_URL_SIZE - 1] = '\0';
    strncpy(notificationsData->apiSecret, apiSecret, NOTIFICATIONS_API_SECRET_SIZE);
    notificationsData->apiSecret[NOTIFICATIONS_API_SECRET_SIZE - 1] = '\0';

    saveNotificationsData(notificationsData);
    free(notificationsData);
    request->send(200);
  }

  static void test(AsyncWebServerRequest *request)
  {
    NotificationsData *notificationsData = loadNotificationsData();
    if (!notificationsData->isSet)
    {
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
      free(notificationsData);
      return;
    }

    // Build args on the heap — the handler returns immediately so stack is gone
    TestNotificationTaskArgs *args = (TestNotificationTaskArgs *)malloc(sizeof(TestNotificationTaskArgs));
    if (!args)
    {
      free(notificationsData);
      request->send(500, "application/json", "{\"error\":\"OUT_OF_MEMORY\"}");
      return;
    }

    // Build URL
    etl::string<NOTIFICATIONS_API_URL_SIZE + 32> url;
    url = notificationsData->apiUrl;
    if (!url.empty() && url.back() == '/')
      url += "api/subscriptions/test";
    else
      url += "/api/subscriptions/test";

    strncpy(args->url, url.c_str(), sizeof(args->url) - 1);
    args->url[sizeof(args->url) - 1] = '\0';
    strncpy(args->apiSecret, notificationsData->apiSecret, sizeof(args->apiSecret) - 1);
    args->apiSecret[sizeof(args->apiSecret) - 1] = '\0';

    free(notificationsData);

    xTaskCreateStatic(
        testNotificationTask,
        "http_worker",
        8192,
        args,
        1,
        xStack,
        &xTaskBuffer);

    request->send(200);
  }
};