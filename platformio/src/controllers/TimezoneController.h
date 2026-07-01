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
#include "preferences/Timezone.h"

class TimezoneController
{
public:
  static void getConfig(AsyncWebServerRequest *request)
  {
    TimezoneData *timezoneData = loadTimezoneData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto config = root["config"].to<ArduinoJson::JsonObject>();

    root["isSet"] = timezoneData->isSet;
    config["timezonePosix"] = timezoneData->timezonePosix;
    if (timezoneData->isSet)
    {
      config["iana"] = timezoneData->iana;
    }

    response->setLength();
    request->send(response);
  }

  static void setConfig(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["timezonePosix"].is<const char *>() ||
        !json["iana"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *timezonePosix = json["timezonePosix"].as<const char *>();
    const char *iana = json["iana"].as<const char *>();

    TimezoneData *timezoneData = loadTimezoneData();

    timezoneData->isSet = true;
    strlcpy(timezoneData->timezonePosix, timezonePosix, TIMEZONE_POSIX_SIZE);
    strlcpy(timezoneData->iana, iana, TIMEZONE_IANA_SIZE);

    saveTimezoneData(timezoneData);
    request->send(200);

    delayedRestart();
  }
};