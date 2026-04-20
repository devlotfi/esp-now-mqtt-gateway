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
#include "preferences/Mqtt.h"

class MqttController
{
public:
  static void getConfig(AsyncWebServerRequest *request)
  {
    MqttData *mqttData = loadMqttData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto config = root["config"].to<ArduinoJson::JsonObject>();

    root["isSet"] = mqttData->isSet;
    if (mqttData->isSet)
    {
      config["useAuth"] = mqttData->useAuth;
      config["clientId"] = mqttData->clientId;
      config["username"] = mqttData->username;
      config["password"] = mqttData->password;
      config["url"] = mqttData->url;
    }

    free(mqttData);
    response->setLength();
    request->send(response);
  }

  static void setConfig(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["clientId"].is<const char *>() ||
        !json["url"].is<const char *>() ||
        !json["useAuth"].is<bool>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *clientId = json["clientId"].as<const char *>();
    const char *url = json["url"].as<const char *>();
    if (!isValidMqttUrl(url))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    bool useAuth = json["useAuth"].as<bool>();
    if (useAuth && (!json["username"].is<const char *>() || !json["password"].is<const char *>()))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    MqttData *mqttData = loadMqttData();

    mqttData->isSet = true;
    strncpy(mqttData->clientId, clientId, MQTT_CLIENT_ID_SIZE);
    strncpy(mqttData->url, url, MQTT_URL_SIZE);
    if (useAuth)
    {
      const char *username = json["username"].as<const char *>();
      const char *password = json["password"].as<const char *>();
      mqttData->useAuth = true;
      strncpy(mqttData->username, username, MQTT_USERNAME_SIZE);
      strncpy(mqttData->password, password, MQTT_PASSWORD_SIZE);
    }
    else
    {
      mqttData->useAuth = false;
    }

    saveMqttData(mqttData);
    free(mqttData);
    request->send(200);

    delayedRestart();
  }
};