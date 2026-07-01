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
#include "preferences/Peer.h"
#include "preferences/EspNow.h"
#include "Lookup.h"
#include "EspNow.h"

class PeerController
{
public:
  static void peers(AsyncWebServerRequest *request)
  {
    PeerData *peerData = loadPeerData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto list = root["peers"].to<ArduinoJson::JsonArray>();

    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
      auto peerJson = list.add<ArduinoJson::JsonObject>();
      peerJson["id"] = peer.id;
      peerJson["name"] = peer.name;
      char macStr[MAC_SIZE_STRING];
      macBytesToString(peer.mac, macStr);
      peerJson["mac"] = macStr;
      char lmkStr[ESP_NOW_KEY_SIZE_STRING];
      keyBytesToHex(peer.lmk, lmkStr);
      peerJson["lmk"] = lmkStr;
    }

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
    PeerData *peerData = loadPeerData();

    // check existing
    if (peerData->peerCount >= PEER_LIST_SIZE)
    {
      request->send(500, "application/json", "{\"error\":\"MAX_PEERS_REACHED\"}");
      return;
    }
    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
      char macStrItem[18];
      macBytesToString(peer.mac, macStrItem);
      if (strcmp(peer.name, name) == 0 || strcmp(macStrItem, macStr) == 0)
      {
        request->send(403, "application/json", "{\"error\":\"EXISTS\"}");
        return;
      }
    }

    // saving in NVS
    Peer peer;
    strlcpy(peer.name, name, NAME_SIZE - 1);
    uuid(peer.id);
    macStringToBytes(macStr, peer.mac);
    keyHexToBytes(lmkStr, peer.lmk);
    peer.topicCount = 0;

    peerData->peerList[peerData->peerCount] = peer;
    peerData->peerCount++;
    savePeerData(peerData);

    // esp-now
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = espNowData->channel;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, peer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, peer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("ESP-NOW: Cannot add peer");
    }
    topicSet.init(peerData);
    topicToMacsMap.init(peerData);
    request->send(200);
  }

  static void deletePeer(AsyncWebServerRequest *request)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    PeerData *peerData = loadPeerData();
    bool found = false;

    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      if (strcmp(peerData->peerList[i].id, id) == 0)
      {
        esp_err_t del_status = esp_now_del_peer(peerData->peerList[i].mac);
        if (del_status != ESP_OK)
        {
          Serial.println("Missing peer in esp-now while deleting");
        }

        for (size_t j = i; j < (size_t)peerData->peerCount - 1; j++)
        {
          peerData->peerList[j] = peerData->peerList[j + 1];
        }

        peerData->peerCount--;
        found = true;
        break;
      }
    }

    if (found)
    {
      savePeerData(peerData);
      topicSet.init(peerData);
      topicToMacsMap.init(peerData);
      request->send(200);
    }
    else
    {
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
  }

  static void topics(AsyncWebServerRequest *request)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    bool found = false;
    PeerData *peerData = loadPeerData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();

    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
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

    PeerData *peerData = loadPeerData();
    bool found = false;

    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
      if (strcmp(peer.id, id) == 0)
      {
        found = true;

        if (peer.topicCount >= TOPIC_LIST_SIZE)
        {
          request->send(500, "application/json", "{\"error\":\"MAX_TOPICS_REACHED\"}");
          return;
        }

        for (size_t j = 0; j < peer.topicCount; j++)
        {
          if (strcmp(peer.topicList[j], topic) == 0)
          {
            request->send(403, "application/json", "{\"error\":\"EXISTS\"}");
            return;
          }
        }

        strlcpy(peer.topicList[peer.topicCount], topic, TOPIC_SIZE - 1);
        peer.topicCount++;
        esp_mqtt_client_subscribe(mqttClient, topic, 1);
        break;
      }
    }

    if (found)
    {
      savePeerData(peerData);
      topicSet.init(peerData);
      topicToMacsMap.init(peerData);
      request->send(200);
    }
    else
    {
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

    PeerData *peerData = loadPeerData();
    bool peerFound = false;
    bool topicFound = false;

    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
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
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
      return;
    }

    // if no subscribers unsubscribe
    bool topicExists = false;
    for (size_t i = 0; i < peerData->peerCount; i++)
    {
      auto &peer = peerData->peerList[i];
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

    savePeerData(peerData);
    topicSet.init(peerData);
    topicToMacsMap.init(peerData);
    request->send(200);
  }
};