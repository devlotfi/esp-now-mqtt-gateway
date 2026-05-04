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
#include "preferences/SleepyPeer.h"
#include "Lookup.h"
#include "EspNow.h"

class SleepyPeerController
{
public:
  static void sleepyPeers(AsyncWebServerRequest *request)
  {
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot().to<JsonObject>();
    auto list = root["sleepyPeers"].to<ArduinoJson::JsonArray>();

    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      auto sleepyPeerJson = list.add<ArduinoJson::JsonObject>();
      sleepyPeerJson["id"] = sleepyPeer.id;
      sleepyPeerJson["name"] = sleepyPeer.name;
      char macStr[MAC_SIZE_STRING];
      macBytesToString(sleepyPeer.mac, macStr);
      sleepyPeerJson["mac"] = macStr;
      char lmkStr[ESP_NOW_KEY_SIZE_STRING];
      keyBytesToHex(sleepyPeer.lmk, lmkStr);
      sleepyPeerJson["lmk"] = lmkStr;
      sleepyPeerJson["commandTopic"] = sleepyPeer.commandTopic;
      sleepyPeerJson["dataTopic"] = sleepyPeer.dataTopic;
    }

    response->setLength();
    request->send(response);
  }

  static void addSleepyPeer(AsyncWebServerRequest *request, ArduinoJson::JsonVariant &json)
  {
    if (
        !json["name"].is<const char *>() ||
        !json["mac"].is<const char *>() ||
        !json["lmk"].is<const char *>() ||
        !json["commandTopic"].is<const char *>() ||
        !json["dataTopic"].is<const char *>())
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }
    const char *name = json["name"].as<const char *>();
    const char *macStr = json["mac"].as<const char *>();
    const char *lmkStr = json["lmk"].as<const char *>();
    const char *commandTopic = json["commandTopic"].as<const char *>();
    const char *dataTopic = json["dataTopic"].as<const char *>();
    if (!isValidEspNowKey(lmkStr) || !isUnicastMacAddress(macStr))
    {
      request->send(400, "application/json", "{\"error\":\"INVALID_REQUEST\"}");
      return;
    }

    EspNowData *espNowData = loadEspNowData();
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();

    // check existing
    if (sleepyPeerData->sleepyPeerCount >= PEER_LIST_SIZE)
    {
      request->send(500, "application/json", "{\"error\":\"MAX_PEERS_REACHED\"}");
      return;
    }
    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &peer = sleepyPeerData->sleepyPeerList[i];
      char macStrItem[18];
      macBytesToString(peer.mac, macStrItem);
      if (strcmp(peer.name, name) == 0 || strcmp(macStrItem, macStr) == 0)
      {
        request->send(403, "application/json", "{\"error\":\"EXISTS\"}");
        return;
      }
    }

    // saving in NVS
    SleepyPeer sleepyPeer;
    strncpy(sleepyPeer.name, name, NAME_SIZE - 1);
    sleepyPeer.name[NAME_SIZE - 1] = '\0';
    uuid(sleepyPeer.id);
    macStringToBytes(macStr, sleepyPeer.mac);
    keyHexToBytes(lmkStr, sleepyPeer.lmk);
    strncpy(sleepyPeer.commandTopic, commandTopic, TOPIC_SIZE - 1);
    sleepyPeer.commandTopic[TOPIC_SIZE - 1] = '\0';
    strncpy(sleepyPeer.dataTopic, dataTopic, TOPIC_SIZE - 1);
    sleepyPeer.dataTopic[TOPIC_SIZE - 1] = '\0';

    sleepyPeerData->sleepyPeerList[sleepyPeerData->sleepyPeerCount] = sleepyPeer;
    sleepyPeerData->sleepyPeerCount++;
    saveSleepyPeerData(sleepyPeerData);

    // esp-now
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = espNowData->channel;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, sleepyPeer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, sleepyPeer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("ESP-NOW: Cannot add sleepy peer");
    }
    // topicSet.init(sleepyPeerData);
    // topicToMacsMap.init(sleepyPeerData);
    request->send(200);
  }

  static void deleteSleepyPeer(AsyncWebServerRequest *request)
  {
    String idStr = request->pathArg(0);
    const char *id = idStr.c_str();

    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();
    bool found = false;

    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      if (strcmp(sleepyPeerData->sleepyPeerList[i].id, id) == 0)
      {
        esp_err_t del_status = esp_now_del_peer(sleepyPeerData->sleepyPeerList[i].mac);
        if (del_status != ESP_OK)
        {
          Serial.println("Missing peer in esp-now while deleting");
        }

        for (size_t j = i; j < (size_t)sleepyPeerData->sleepyPeerCount - 1; j++)
        {
          sleepyPeerData->sleepyPeerList[j] = sleepyPeerData->sleepyPeerList[j + 1];
        }

        sleepyPeerData->sleepyPeerCount--;
        found = true;
        break;
      }
    }

    if (found)
    {
      saveSleepyPeerData(sleepyPeerData);
      // topicSet.init(sleepyPeerData);
      // topicToMacsMap.init(sleepyPeerData);
      request->send(200);
    }
    else
    {
      request->send(404, "application/json", "{\"error\":\"NOT_FOUND\"}");
    }
  }
};