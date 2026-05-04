#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "preferences/EspNow.h"
#include "Lookup.h"
#include "EspNow.h"

class EspNowController
{
public:
  static void getConfig(AsyncWebServerRequest *request)
  {
    EspNowData *espNowData = loadEspNowData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    char macStr[MAC_SIZE_STRING];
    macBytesToString(espNowData->mac, macStr);
    root["mac"] = macStr;
    root["channel"] = espNowData->channel;
    root["pmkSet"] = espNowData->pmkSet;
    if (espNowData->pmkSet) // pmk is optional
    {
      char pmkStr[ESP_NOW_KEY_SIZE_STRING];
      keyBytesToHex(espNowData->pmk, pmkStr);
      root["pmk"] = pmkStr;
    }

    response->setLength();
    request->send(response);
  }

  static void setConfig(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["mac"].is<const char *>() ||
        !json["channel"].is<uint8_t>() ||
        !json["pmk"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *macStr = json["mac"].as<const char *>();
    uint8_t channel = json["channel"].as<uint8_t>();
    const char *pmkStr = json["pmk"].as<const char *>();
    if (!isValidWifiChannel(channel) || !isUnicastMacAddress(macStr) || !isValidEspNowKey(pmkStr))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    EspNowData *espNowData = loadEspNowData();

    macStringToBytes(macStr, espNowData->mac);
    espNowData->channel = channel;
    espNowData->pmkSet = true;
    keyHexToBytes(pmkStr, espNowData->pmk);

    saveEspNowData(espNowData);
    request->send(200);

    delayedRestart();
  }
};