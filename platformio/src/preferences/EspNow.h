#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct EspNowData
{
  uint8_t channel;
  uint8_t mac[MAC_SIZE_BYTES];
  bool pmkSet;
  uint8_t pmk[ESP_NOW_KEY_SIZE_BYTES];
};

static EspNowData *espNowDataCache = nullptr;

void saveEspNowData(const EspNowData *data)
{
  size_t written = preferences.putBytes("esp_now_data", data, sizeof(EspNowData));
  if (written != sizeof(EspNowData))
    return;
  if (!espNowDataCache)
    espNowDataCache = (EspNowData *)malloc(sizeof(EspNowData));
  if (espNowDataCache)
    memcpy(espNowDataCache, data, sizeof(EspNowData));
}

EspNowData *loadEspNowData()
{
  if (espNowDataCache)
    return espNowDataCache;

  espNowDataCache = (EspNowData *)malloc(sizeof(EspNowData));
  if (!espNowDataCache)
    return nullptr;
  espNowDataCache->channel = 1;
  espNowDataCache->pmkSet = false;
  memcpy(espNowDataCache->mac, defaultMac, MAC_SIZE_BYTES);
  preferences.getBytes("esp_now_data", espNowDataCache, sizeof(EspNowData));
  return espNowDataCache;
}