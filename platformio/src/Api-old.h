#pragma once

#include <ArduinoJson.h>
#include <esp_now.h>
#include <uri/UriBraces.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "StaticBufferAllocator.h"
#include "Data.h"

static uint8_t requestBuffer[8192];
static uint8_t responseBuffer[8192];
StaticBufferAllocator requestAllocator(requestBuffer, sizeof(requestBuffer));
StaticBufferAllocator responseAllocator(responseBuffer, sizeof(responseBuffer));
static char response[8192];
static EspNowData espNowData;

void handleIndex()
{
  server.send(200);
}

void handleLogin()
{
  ArduinoJson::JsonDocument requestDoc(&requestAllocator);
  ArduinoJson::JsonDocument responseDoc(&responseAllocator);
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(requestDoc, server.arg("plain"));
  if (jsonError)
  {
    server.send(400);
    return;
  }
  if (
      !requestDoc["password"].is<const char *>())
  {
    server.send(400);
    return;
  }

  const char *password = requestDoc["password"].as<const char *>();
  AuthData authData = loadAuthData();
  if (!verifyPassword(password, authData.passwordSalt, authData.passwordHash))
  {
    server.send(403);
    return;
  }

  char token[256];
  generateToken(token, sizeof(token));
  responseDoc["token"] = token;
  ArduinoJson::serializeJson(responseDoc, response, sizeof(response));
  server.send(200, "application/json", response);
}

void handleChangePassword()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  ArduinoJson::JsonDocument requestDoc(&requestAllocator);
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(requestDoc, server.arg("plain"));
  if (jsonError)
  {
    server.send(400);
    return;
  }
  if (
      !requestDoc["password"].is<const char *>())
  {
    server.send(400);
    return;
  }

  const char *password = requestDoc["password"].as<const char *>();
  AuthData authData;
  uint8_t salt[PASSWORD_SALT_SIZE];
  uint8_t hash[PASSWORD_HASH_SIZE];
  hashPassword(password, salt, hash);
  memcpy(authData.passwordSalt, salt, PASSWORD_SALT_SIZE);
  memcpy(authData.passwordHash, hash, PASSWORD_HASH_SIZE);
  saveAuthData(authData);

  server.send(200);
}

void handleChangePMK()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  ArduinoJson::JsonDocument requestDoc(&requestAllocator);
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(requestDoc, server.arg("plain"));
  if (jsonError)
  {
    server.send(400);
    return;
  }
  if (
      !requestDoc["pmk"].is<const char *>())
  {
    server.send(400);
    return;
  }
  const char *pmkStr = requestDoc["pmk"].as<const char *>();
  if (!isValidEspNowKey(pmkStr))
  {
    server.send(400);
    return;
  }

  loadEspNowData(espNowData);
  espNowData.pmkSet = true;
  keyHexToBytes(pmkStr, espNowData.pmk);
  saveEspNowData(espNowData);

  esp_err_t result = esp_now_set_pmk(espNowData.pmk);
  if (result != ESP_OK)
  {
    server.send(500);
    return;
  }

  server.send(200);
}

void handlePeerList()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  ArduinoJson::JsonDocument responseDoc(&responseAllocator);
  auto list = responseDoc["peers"].to<ArduinoJson::JsonArray>();

  loadEspNowData(espNowData);
  for (size_t i = 0; i < espNowData.peerCount; i++)
  {
    auto &peer = espNowData.peerList[i];
    auto peerJson = list.add<ArduinoJson::JsonObject>();
    peerJson["id"] = peer.id;
    peerJson["name"] = peer.name;
    char macStr[18];
    macBytesToString(peer.mac, macStr);
    peerJson["mac"] = macStr;
  }

  ArduinoJson::serializeJson(responseDoc, response, sizeof(response));
  server.send(200, "application/json", response);
}

void handleAddPeer()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  ArduinoJson::JsonDocument requestDoc(&requestAllocator);
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(requestDoc, server.arg("plain"));
  if (jsonError)
  {
    server.send(400);
    return;
  }
  if (
      !requestDoc["name"].is<const char *>() ||
      !requestDoc["mac"].is<const char *>() ||
      !requestDoc["lmk"].is<const char *>())
  {
    server.send(400);
    return;
  }
  const char *name = requestDoc["name"].as<const char *>();
  const char *macStr = requestDoc["mac"].as<const char *>();
  const char *lmkStr = requestDoc["lmk"].as<const char *>();
  if (!isValidEspNowKey(lmkStr) || !isUnicastMacAddress(macStr))
  {
    server.send(400);
    return;
  }

  loadEspNowData(espNowData);
  if (espNowData.peerCount >= PEER_LIST_SIZE)
  {
    server.send(500);
    return;
  }
  for (size_t i = 0; i < espNowData.peerCount; i++)
  {
    auto &peer = espNowData.peerList[i];
    char macStrItem[18];
    macBytesToString(peer.mac, macStrItem);
    if (strcmp(peer.name, name) == 0 || strcmp(macStrItem, macStr) == 0)
    {
      server.send(403);
      return;
    }
  }

  Peer peer;
  strncpy(peer.name, name, NAME_SIZE);
  uuid(peer.id);
  macStringToBytes(macStr, peer.mac);
  keyHexToBytes(lmkStr, peer.lmk);
  peer.topicCount = 0;
  espNowData.peerList[espNowData.peerCount] = peer;
  espNowData.peerCount++;
  saveEspNowData(espNowData);

  // esp-now
  esp_now_peer_info_t peerInfo{};
  peerInfo.channel = 1;
  peerInfo.encrypt = true;
  memcpy(peerInfo.peer_addr, peer.mac, MAC_SIZE_BYTES);
  memcpy(peerInfo.lmk, peer.lmk, ESP_NOW_KEY_SIZE_BYTES);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    server.send(500);
    return;
  }

  server.send(200);
}

void handleDeletePeer()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  // Read ID from the URL path: /peers/{id}
  String idStr = server.pathArg(0);
  const char *id = idStr.c_str();

  loadEspNowData(espNowData);
  bool found = false;

  for (size_t i = 0; i < espNowData.peerCount; i++)
  {
    if (strcmp(espNowData.peerList[i].id, id) == 0)
    {
      for (size_t j = i; j < (size_t)espNowData.peerCount - 1; j++)
      {
        espNowData.peerList[j] = espNowData.peerList[j + 1];
      }
      esp_err_t del_status = esp_now_del_peer(espNowData.peerList[i].mac);
      if (del_status == ESP_OK)
      {
        server.send(200);
        return;
      }
      espNowData.peerCount--;
      found = true;
      break;
    }
  }

  if (found)
  {
    saveEspNowData(espNowData);
    server.send(200);
  }
  else
  {
    server.send(404);
  }
}

void handleTopicList()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  // Read ID from the URL path: /peers/{id}/topics
  String idStr = server.pathArg(0);
  const char *id = idStr.c_str();

  ArduinoJson::JsonDocument responseDoc(&responseAllocator);
  loadEspNowData(espNowData);
  bool found = false;

  for (size_t i = 0; i < espNowData.peerCount; i++)
  {
    auto &peer = espNowData.peerList[i];
    if (strcmp(peer.id, id) == 0)
    {
      found = true;
      auto topicList = responseDoc["topicList"].to<ArduinoJson::JsonArray>();

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
    ArduinoJson::serializeJson(responseDoc, response, sizeof(response));
    server.send(200, "application/json", response);
  }
  else
  {
    server.send(404);
  }
}

void handleSetTopicList()
{
  auto auth = checkAuthorization(server);
  if (!auth)
    return;

  // Read ID from the URL path: /peers/{id}/topics
  String idStr = server.pathArg(0);
  const char *id = idStr.c_str();

  ArduinoJson::JsonDocument requestDoc(&requestAllocator);
  ArduinoJson::DeserializationError jsonError = ArduinoJson::deserializeJson(requestDoc, server.arg("plain"));

  if (jsonError || !requestDoc["topicList"].is<ArduinoJson::JsonArray>())
  {
    server.send(400);
    return;
  }

  auto topicListJson = requestDoc["topicList"].as<ArduinoJson::JsonArray>();
  char topicList[TOPIC_LIST_SIZE][TOPIC_SIZE];
  bool allStrings = true;
  size_t topicCount = 0;

  for (auto v : topicListJson)
  {
    // FIX: Prevent array out-of-bounds memory corruption
    if (topicCount >= TOPIC_LIST_SIZE)
    {
      break;
    }

    if (!v.is<const char *>())
    {
      allStrings = false;
      break;
    }
    else
    {
      strncpy(topicList[topicCount], v.as<const char *>(), TOPIC_SIZE - 1);
      topicList[topicCount][TOPIC_SIZE - 1] = '\0'; // Ensure null-termination
    }
    topicCount++;
  }

  if (!allStrings)
  {
    server.send(400);
    return;
  }

  loadEspNowData(espNowData);
  bool found = false;
  for (size_t i = 0; i < espNowData.peerCount; i++)
  {
    auto &peer = espNowData.peerList[i];
    if (strcmp(peer.id, id) == 0)
    {
      found = true;
      memcpy(peer.topicList, topicList, sizeof(topicList));
      peer.topicCount = topicCount;
      break;
    }
  }

  if (found)
  {
    saveEspNowData(espNowData);
    server.send(200);
  }
  else
  {
    server.send(404);
  }
}

void handleNotFound()
{
  server.send(404, "application/json", "Not Found");
}

void setupEndpoints()
{
  server.enableCORS();
  server.on("/", HTTP_GET, handleIndex);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/change-password", HTTP_POST, handleChangePassword);
  server.on("/change-pmk", HTTP_POST, handleChangePMK);
  server.on("/peers", HTTP_GET, handlePeerList);
  server.on("/peers", HTTP_POST, handleAddPeer);
  server.on(UriBraces("/peers/{}"), HTTP_DELETE, handleDeletePeer);
  server.on(UriBraces("/peers/{}/topics"), HTTP_GET, handleTopicList);
  server.on(UriBraces("/peers/{}/topics"), HTTP_POST, handleSetTopicList);

  server.onNotFound(handleNotFound);
}