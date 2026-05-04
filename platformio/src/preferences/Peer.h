#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct Peer
{
  char id[UUID_V4_SIZE];
  char name[NAME_SIZE];
  uint8_t mac[MAC_SIZE_BYTES];
  uint8_t lmk[ESP_NOW_KEY_SIZE_BYTES];
  uint8_t topicCount;
  char topicList[TOPIC_LIST_SIZE][TOPIC_SIZE];
};

struct PeerData
{
  uint8_t peerCount;
  Peer peerList[PEER_LIST_SIZE];
};

static PeerData *peerDataCache = nullptr;

void savePeerData(const PeerData *data)
{
  size_t written = preferences.putBytes("peer_data", data, sizeof(PeerData));
  if (written != sizeof(PeerData))
    return;
  if (!peerDataCache)
    peerDataCache = (PeerData *)malloc(sizeof(PeerData));
  if (peerDataCache)
    memcpy(peerDataCache, data, sizeof(PeerData));
}

PeerData *loadPeerData()
{
  if (peerDataCache)
    return peerDataCache;

  peerDataCache = (PeerData *)malloc(sizeof(PeerData));
  if (!peerDataCache)
    return nullptr;
  peerDataCache->peerCount = 0;
  preferences.getBytes("peer_data", peerDataCache, sizeof(PeerData));
  return peerDataCache;
}