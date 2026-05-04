#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct SleepyPeer
{
  char id[UUID_V4_SIZE];
  char name[NAME_SIZE];
  uint8_t mac[MAC_SIZE_BYTES];
  uint8_t lmk[ESP_NOW_KEY_SIZE_BYTES];
  char commandTopic[TOPIC_SIZE];
  char dataTopic[TOPIC_SIZE];
};

struct SleepyPeerData
{
  uint8_t sleepyPeerCount;
  SleepyPeer sleepyPeerList[SLEEPY_PEER_LIST_SIZE];
};

static SleepyPeerData *sleepyPeerDataCache = nullptr;

void saveSleepyPeerData(const SleepyPeerData *data)
{
  size_t written = preferences.putBytes("epy_peer_data", data, sizeof(SleepyPeerData));
  if (written != sizeof(SleepyPeerData))
    return;
  if (!sleepyPeerDataCache)
    sleepyPeerDataCache = (SleepyPeerData *)malloc(sizeof(SleepyPeerData));
  if (sleepyPeerDataCache)
    memcpy(sleepyPeerDataCache, data, sizeof(SleepyPeerData));
}

SleepyPeerData *loadSleepyPeerData()
{
  if (sleepyPeerDataCache)
    return sleepyPeerDataCache;

  sleepyPeerDataCache = (SleepyPeerData *)malloc(sizeof(SleepyPeerData));
  if (!sleepyPeerDataCache)
    return nullptr;
  sleepyPeerDataCache->sleepyPeerCount = 0;
  preferences.getBytes("epy_peer_data", sleepyPeerDataCache, sizeof(SleepyPeerData));
  return sleepyPeerDataCache;
}