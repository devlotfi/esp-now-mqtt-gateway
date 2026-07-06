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

static AuthData *authDataCache = nullptr;

void saveAuthData(const AuthData *data)
{
  size_t written = preferences.putBytes("auth_data", data, sizeof(AuthData));
  if (written != sizeof(AuthData))
    return;
  if (!authDataCache)
    authDataCache = (AuthData *)heap_caps_malloc(sizeof(AuthData), MALLOC_CAP_SPIRAM);
  if (authDataCache)
    memcpy(authDataCache, data, sizeof(AuthData));
}

AuthData *loadAuthData()
{
  if (authDataCache)
    return authDataCache;

  authDataCache = (AuthData *)heap_caps_malloc(sizeof(AuthData), MALLOC_CAP_SPIRAM);
  if (!authDataCache)
    return nullptr;
  uint8_t salt[PASSWORD_SALT_SIZE];
  uint8_t hash[PASSWORD_HASH_SIZE];
  hashPassword("admin", salt, hash);
  memcpy(authDataCache->passwordSalt, salt, PASSWORD_SALT_SIZE);
  memcpy(authDataCache->passwordHash, hash, PASSWORD_HASH_SIZE);
  preferences.getBytes("auth_data", authDataCache, sizeof(AuthData));
  return authDataCache;
}