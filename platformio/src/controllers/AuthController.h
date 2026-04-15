#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <uri/UriBraces.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "NvsData.h"
#include "Lookup.h"
#include "EspNow.h"

class AuthController
{
public:
  static void index(AsyncWebServerRequest *request)
  {
    request->send(200);
  }

  static void login(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["password"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    const char *password = json["password"].as<const char *>();
    AuthData *authData = loadAuthData();
    if (!verifyPassword(password, authData->passwordSalt, authData->passwordHash))
    {
      free(authData);
      request->send(403, "application/json", "{\"error\":\"WRONG_PASSWORD\"}");
      return;
    }

    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    char token[256];
    generateToken(token, sizeof(token));
    root["token"] = token;

    free(authData);
    response->setLength();
    request->send(response);
  }

  static void setPassword(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["password"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    const char *password = json["password"].as<const char *>();
    AuthData authData;
    uint8_t salt[PASSWORD_SALT_SIZE];
    uint8_t hash[PASSWORD_HASH_SIZE];
    hashPassword(password, salt, hash);
    memcpy(authData.passwordSalt, salt, PASSWORD_SALT_SIZE);
    memcpy(authData.passwordHash, hash, PASSWORD_HASH_SIZE);
    saveAuthData(&authData);
    request->send(200);
  }

  static void setPMK(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["pmk"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *pmkStr = json["pmk"].as<const char *>();
    if (!isValidEspNowKey(pmkStr))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    EspNowData *espNowData = loadEspNowData();
    espNowData->pmkSet = true;
    keyHexToBytes(pmkStr, espNowData->pmk);
    saveEspNowData(espNowData);

    esp_err_t result = esp_now_set_pmk(espNowData->pmk);
    if (result != ESP_OK)
    {
      free(espNowData);
      request->send(500, "application/json", "{\"error\":\"CANNOT_SET_PMK_TO_ESP_NOW\"}");
      return;
    }

    clearPeers();
    initPeers();

    free(espNowData);
    request->send(200);
  }
};