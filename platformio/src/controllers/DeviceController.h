#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include "esp_mac.h"
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "Lookup.h"
#include "EspNow.h"

class DeviceController
{
public:
  static void reboot(AsyncWebServerRequest *request)
  {
    request->send(200);
    delayedRestart();
  }

  static void status(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    auto freeHeap = ESP.getFreeHeap();
    auto heapSize = ESP.getHeapSize();
    auto minFreeHeap = ESP.getMinFreeHeap();
    auto maxAllocHeap = ESP.getMaxAllocHeap();

    auto freePsram = ESP.getFreePsram();
    auto psramSize = ESP.getPsramSize();
    auto minFreePsram = ESP.getMinFreePsram();
    auto maxAllocPsram = ESP.getMaxAllocPsram();

    auto flashChipSize = ESP.getFlashChipSize();
    auto freeSketchSpace = ESP.getFreeSketchSpace();

    auto localIP = ETH.localIP().toString();
    auto macAddress = ETH.macAddress();
    auto linkUp = ETH.linkUp();
    auto linkSpeed = ETH.linkSpeed();
    auto fullDuplex = ETH.fullDuplex();

    auto uptime = millis();

    float cpuTemp;
    bool tempReading = false;
    if (temperature_sensor_get_celsius(temp_sensor, &cpuTemp) == ESP_OK)
      tempReading = true;

    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    auto heapObj = root["heap"].to<ArduinoJson::JsonObject>();
    heapObj["freeHeap"] = freeHeap;
    heapObj["heapSize"] = heapSize;
    heapObj["minFreeHeap"] = minFreeHeap;
    heapObj["maxAllocHeap"] = maxAllocHeap;

    auto psramObj = root["psram"].to<ArduinoJson::JsonObject>();
    psramObj["freePsram"] = freePsram;
    psramObj["psramSize"] = psramSize;
    psramObj["minFreePsram"] = minFreePsram;
    psramObj["maxAllocPsram"] = maxAllocPsram;

    auto storageObj = root["storage"].to<ArduinoJson::JsonObject>();
    storageObj["flashChipSize"] = flashChipSize;
    storageObj["freeSketchSpace"] = freeSketchSpace;

    auto ethernetObj = root["ethernet"].to<ArduinoJson::JsonObject>();
    ethernetObj["localIP"] = localIP;
    ethernetObj["macAddress"] = macAddress;
    ethernetObj["linkUp"] = linkUp;
    ethernetObj["linkSpeed"] = linkSpeed;
    ethernetObj["fullDuplex"] = fullDuplex;

    auto generalObj = root["general"].to<ArduinoJson::JsonObject>();
    generalObj["uptime"] = uptime;
    generalObj["mqttConnected"] = mqttConnected;
    if (tempReading)
    {
      generalObj["cpuTemp"] = cpuTemp;
    }
    else
    {
      generalObj["cpuTemp"] = nullptr;
    }

    response->setLength();
    request->send(response);
  }
};