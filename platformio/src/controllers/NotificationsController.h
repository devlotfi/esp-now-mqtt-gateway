#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <etl/string.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "preferences/Notifications.h"
#include "Lookup.h"
#include "EspNow.h"
#include "Notifications.h"

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

    testNotification(notificationsData);
    free(notificationsData);
    request->send(200);
  }
};