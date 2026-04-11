#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct AuthData
{
  uint8_t passwordSalt[PASSWORD_SALT_SIZE];
  uint8_t passwordHash[PASSWORD_HASH_SIZE];
};

void saveAuthData(const AuthData *data)
{
  preferences.putBytes("auth_data", data, sizeof(AuthData));
}

AuthData *loadAuthData()
{
  AuthData *data = (AuthData *)malloc(sizeof(AuthData));
  if (!data)
    return nullptr;
  uint8_t salt[PASSWORD_SALT_SIZE];
  uint8_t hash[PASSWORD_HASH_SIZE];
  hashPassword("admin", salt, hash);
  memcpy(data->passwordSalt, salt, PASSWORD_SALT_SIZE);
  memcpy(data->passwordHash, hash, PASSWORD_HASH_SIZE);
  preferences.getBytes("auth_data", data, sizeof(AuthData));
  return data;
}

struct Peer
{
  char id[UUID_V4_SIZE];
  char name[NAME_SIZE];
  uint8_t mac[MAC_SIZE_BYTES];
  uint8_t lmk[ESP_NOW_KEY_SIZE_BYTES];
  uint8_t topicCount;
  char topicList[TOPIC_LIST_SIZE][TOPIC_SIZE];
};

struct EspNowData
{
  bool pmkSet;
  uint8_t pmk[ESP_NOW_KEY_SIZE_BYTES];
  uint8_t peerCount;
  Peer peerList[PEER_LIST_SIZE];
};

void saveEspNowData(const EspNowData *data)
{
  preferences.putBytes("esp_now_data", data, sizeof(EspNowData));
}

EspNowData *loadEspNowData()
{
  EspNowData *data = (EspNowData *)malloc(sizeof(EspNowData));
  if (!data)
    return nullptr;
  data->pmkSet = false;
  data->peerCount = 0;
  preferences.getBytes("esp_now_data", data, sizeof(EspNowData));
  return data;
}