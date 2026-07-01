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
#include "preferences/Grafana.h"

class GrafanaController
{
public:
  static void getConfig(AsyncWebServerRequest *request)
  {
    GrafanaData *grafanaData = loadGrafanaData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto config = root["config"].to<ArduinoJson::JsonObject>();

    root["isSet"] = grafanaData->isSet;
    if (grafanaData->isSet)
    {
      config["url"] = grafanaData->url;
      config["instanceId"] = grafanaData->instanceId;
      config["apiKey"] = grafanaData->apiKey;
    }

    response->setLength();
    request->send(response);
  }

  static void setConfig(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["url"].is<const char *>() ||
        !json["instanceId"].is<const char *>() ||
        !json["apiKey"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *url = json["url"].as<const char *>();
    const char *instanceId = json["instanceId"].as<const char *>();
    const char *apiKey = json["apiKey"].as<const char *>();

    GrafanaData *grafanaData = loadGrafanaData();

    grafanaData->isSet = true;
    strlcpy(grafanaData->url, url, GRAFANA_URL_SIZE);
    strlcpy(grafanaData->instanceId, instanceId, GRAFANA_INSTANCE_ID_SIZE);
    strlcpy(grafanaData->apiKey, apiKey, GRAFANA_API_KEY_SIZE);

    saveGrafanaData(grafanaData);
    request->send(200);
  }
};