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
#include "StaticBufferAllocator.h"
#include "Data.h"
#include "Lookup.h"

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

    free(espNowData);
    request->send(200);
  }
};

class PeerController
{
public:
  static void peers(AsyncWebServerRequest *request)
  {
    EspNowData *espNowData = loadEspNowData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto list = root["peers"].to<ArduinoJson::JsonArray>();

    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      auto peerJson = list.add<ArduinoJson::JsonObject>();
      peerJson["id"] = peer.id;
      peerJson["name"] = peer.name;
      char macStr[MAC_SIZE_STRING];
      macBytesToString(peer.mac, macStr);
      peerJson["mac"] = macStr;
    }

    free(espNowData);
    response->setLength();
    request->send(response);
  }

  static void addPeer(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["name"].is<const char *>() ||
        !json["mac"].is<const char *>() ||
        !json["lmk"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *name = json["name"].as<const char *>();
    const char *macStr = json["mac"].as<const char *>();
    const char *lmkStr = json["lmk"].as<const char *>();
    if (!isValidEspNowKey(lmkStr) || !isUnicastMacAddress(macStr))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    EspNowData *espNowData = loadEspNowData();

    // check existing
    if (espNowData->peerCount >= PEER_LIST_SIZE)
    {
      free(espNowData);
      request->send(500, "application/json", "{\"error\":\"MAX_PEERS_REACHED\"}");
      return;
    }
    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      char macStrItem[18];
      macBytesToString(peer.mac, macStrItem);
      if (strcmp(peer.name, name) == 0 || strcmp(macStrItem, macStr) == 0)
      {
        free(espNowData);
        request->send(403, "application/json", "{\"error\":\"EXISTS\"}");
        return;
      }
    }

    // saving in NVS
    Peer peer;
    strncpy(peer.name, name, NAME_SIZE);
    uuid(peer.id);
    macStringToBytes(macStr, peer.mac);
    keyHexToBytes(lmkStr, peer.lmk);
    peer.topicCount = 0;
    espNowData->peerList[espNowData->peerCount] = peer;
    espNowData->peerCount++;
    saveEspNowData(espNowData);

    // esp-now
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = 1;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, peer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, peer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      free(espNowData);
      Serial.println("ESP-NOW: Cannot add peer");
      return;
    }
    topicSet.init(espNowData);
    topicToMacsMap.init(espNowData);
    free(espNowData);
    request->send(200);
  }

  static void deletePeer(AsyncWebServerRequest *request)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    EspNowData *espNowData = loadEspNowData();
    bool found = false;

    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      if (strcmp(espNowData->peerList[i].id, id) == 0)
      {
        esp_err_t del_status = esp_now_del_peer(espNowData->peerList[i].mac);
        if (del_status != ESP_OK)
        {
          Serial.println("Missing peer in esp-now while deleting");
        }

        for (size_t j = i; j < (size_t)espNowData->peerCount - 1; j++)
        {
          espNowData->peerList[j] = espNowData->peerList[j + 1];
        }

        espNowData->peerCount--;
        found = true;
        break;
      }
    }

    if (found)
    {
      saveEspNowData(espNowData);
      topicSet.init(espNowData);
      topicToMacsMap.init(espNowData);
      free(espNowData);
      request->send(200);
    }
    else
    {
      free(espNowData);
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
  }

  static void topics(AsyncWebServerRequest *request)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    bool found = false;
    EspNowData *espNowData = loadEspNowData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      if (strcmp(peer.id, id) == 0)
      {
        found = true;
        auto topicList = root["topicList"].to<ArduinoJson::JsonArray>();

        for (size_t j = 0; j < peer.topicCount; j++)
        {
          auto &topic = peer.topicList[j];
          topicList.add(topic);
        }
        break;
      }
    }

    free(espNowData);
    if (found)
    {
      response->setLength();
      request->send(response);
    }
    else
    {
      delete response;
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
  }

  static void addTopic(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    if (!json["topic"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *topic = json["topic"].as<const char *>();

    EspNowData *espNowData = loadEspNowData();
    bool found = false;

    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      if (strcmp(peer.id, id) == 0)
      {
        found = true;

        if (peer.topicCount >= TOPIC_LIST_SIZE)
        {
          free(espNowData);
          request->send(500, "application/json", "{\"error\":\"MAX_TOPICS_REACHED\"}");
          return;
        }

        for (size_t j = 0; j < peer.topicCount; j++)
        {
          if (strcmp(peer.topicList[j], topic) == 0)
          {
            free(espNowData);
            request->send(403, "application/json", "{\"error\":\"EXISTS\"}");
            return;
          }
        }

        strncpy(peer.topicList[peer.topicCount], topic, TOPIC_SIZE - 1);
        peer.topicList[peer.topicCount][TOPIC_SIZE - 1] = '\0';
        peer.topicCount++;
        esp_mqtt_client_subscribe(mqttClient, topic, 1);
        break;
      }
    }

    if (found)
    {
      saveEspNowData(espNowData);
      topicSet.init(espNowData);
      topicToMacsMap.init(espNowData);
      free(espNowData);
      request->send(200);
    }
    else
    {
      free(espNowData);
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
  }

  static void deleteTopic(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    if (!json["topic"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *topic = json["topic"].as<const char *>();

    EspNowData *espNowData = loadEspNowData();
    bool peerFound = false;
    bool topicFound = false;

    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      if (strcmp(peer.id, id) == 0)
      {
        peerFound = true;

        for (size_t j = 0; j < peer.topicCount; j++)
        {
          if (strcmp(peer.topicList[j], topic) == 0)
          {
            topic = peer.topicList[j];

            topicFound = true;
            for (size_t k = j; k < (size_t)peer.topicCount - 1; k++)
            {
              memcpy(peer.topicList[k], peer.topicList[k + 1], TOPIC_SIZE);
            }
            peer.topicCount--;
            break;
          }
        }
        break;
      }
    }

    if (!peerFound || !topicFound)
    {
      free(espNowData);
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
      return;
    }

    // if no subscribers unsubscribe
    bool topicExists = false;
    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      for (size_t j = 0; j < peer.topicCount; j++)
      {
        auto &topicCurrent = peer.topicList[j];
        if (strcmp(topicCurrent, topic) == 0)
        {
          topicExists = true;
          break;
        }
      }
      if (!topicExists)
      {
        esp_mqtt_client_unsubscribe(mqttClient, topic);
        break;
      }
    }

    saveEspNowData(espNowData);
    topicSet.init(espNowData);
    topicToMacsMap.init(espNowData);
    free(espNowData);
    request->send(200);
  }
};

static AsyncCorsMiddleware cors;

AsyncMiddlewareFunction jwtAuth([](AsyncWebServerRequest *request, ArMiddlewareNext next)
                                {
  if (!request->hasHeader("Authorization")) {
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
    return;
  }
  String header = request->getHeader("Authorization")->value();
  if (!header.startsWith("Bearer ")) {
    request->send(401);
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
  }
  String token = header.substring(7);
  if (!verifyToken(token.c_str())) {
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
    return;
  }
  request->setAttribute("jwt", token);
  next(); });

void setupServer()
{
  // CORS
  cors.setOrigin("*");
  cors.setMethods("GET, POST, PUT, PATCH, DELETE, OPTIONS");
  cors.setHeaders("Content-Type, Authorization");
  cors.setAllowCredentials(false);
  server.addMiddleware(&cors);

  server.on(AsyncURIMatcher::exact("/login"), HTTP_POST, AuthController::login);
  server.on(AsyncURIMatcher::exact("/set-password"), HTTP_POST, AuthController::setPassword).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/set-pmk"), HTTP_POST, AuthController::setPMK).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/peers"), HTTP_GET, PeerController::peers).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/peers"), HTTP_POST, PeerController::addPeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/peers/([0-9a-fA-F-]{36})$"), HTTP_DELETE, PeerController::deletePeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_GET, PeerController::topics).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_POST, PeerController::addTopic).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_DELETE, PeerController::deleteTopic).addMiddleware(&jwtAuth);

  server.begin();
}