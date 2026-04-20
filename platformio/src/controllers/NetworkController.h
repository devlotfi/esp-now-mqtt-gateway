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
#include "preferences/Network.h"

class NetworkController
{
public:
  static void getConfig(AsyncWebServerRequest *request)
  {
    NetworkData *networkData = loadNetworkData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    root["ipAssignment"] = networkData->ipAssignment.to_string();
    auto staticConfig = root["staticConfig"].to<ArduinoJson::JsonObject>();
    // if static provide static config
    if (networkData->ipAssignment == IPAssignment::STATIC)
    {
      staticConfig["ip"] = IPAddress(networkData->ip).toString();
      staticConfig["gateway"] = IPAddress(networkData->gateway).toString();
      staticConfig["subnet"] = IPAddress(networkData->subnet).toString();
      staticConfig["dns"] = IPAddress(networkData->dns).toString();
    }

    free(networkData);
    response->setLength();
    request->send(response);
  }

  static void setConfig(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["ipAssignment"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *ipAssignmentStr = json["ipAssignment"].as<const char *>();
    IPAssignment ipAssignment = IPAssignment::from_string(ipAssignmentStr);
    if (ipAssignment == IPAssignment::UNKNOWN)
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    if (ipAssignment == IPAssignment::STATIC)
    {
      auto staticConfig = json["staticConfig"].as<ArduinoJson::JsonObject>();
      if (
          !staticConfig ||
          !staticConfig["ip"].is<const char *>() ||
          !staticConfig["gateway"].is<const char *>() ||
          !staticConfig["subnet"].is<const char *>() ||
          !staticConfig["dns"].is<const char *>())
      {
        request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
        return;
      }
      const char *ipStr = staticConfig["ip"].as<const char *>();
      const char *gatewayStr = staticConfig["gateway"].as<const char *>();
      const char *subnetStr = staticConfig["subnet"].as<const char *>();
      const char *dnsStr = staticConfig["dns"].as<const char *>();
      if (
          !isPrivateIPv4(ipStr) ||
          !isPrivateIPv4(gatewayStr) ||
          !isValidSubnetMask(subnetStr) ||
          !isValidIPv4(dnsStr))
      {
        request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
        return;
      }

      IPAddress ip(ipStr);
      IPAddress gateway(gatewayStr);
      IPAddress subnet(subnetStr);
      IPAddress dns(dnsStr);

      NetworkData *networkData = loadNetworkData();

      networkData->ipAssignment = IPAssignment::STATIC;
      networkData->ip = (uint32_t)ip;
      networkData->gateway = (uint32_t)gateway;
      networkData->subnet = (uint32_t)subnet;
      networkData->dns = (uint32_t)dns;

      saveNetworkData(networkData);
      free(networkData);
      request->send(200);

      delayedRestart();
    }
    else if (ipAssignment == IPAssignment::DHCP)
    {
      NetworkData *networkData = loadNetworkData();

      networkData->ipAssignment = IPAssignment::DHCP;

      saveNetworkData(networkData);
      free(networkData);
      request->send(200);

      delayedRestart();
    }
  }
};