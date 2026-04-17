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