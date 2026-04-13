#pragma once

#include <stdint.h>
#include <stddef.h>
#include "Properties.h"
#include "Data.h"

class TopicSet
{
public:
  size_t count = 0;
  char set[TOPIC_LIST_SIZE * PEER_LIST_SIZE][TOPIC_SIZE];

  void clear()
  {
    count = 0;
  }

  void init(EspNowData *espNowData)
  {
    clear();
    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      for (size_t j = 0; j < peer.topicCount; j++)
      {
        auto &topic = peer.topicList[j];
        bool exists = false;
        for (size_t k = 0; k < count; k++)
        {
          if (strcmp(set[k], topic) == 0)
          {
            exists = true;
            break;
          }
        }

        if (!exists)
        {
          strcpy(set[count], topic);
          count++;
        }
      }
    }
  }
};

class MacSet
{
public:
  size_t count = 0;
  uint8_t set[PEER_LIST_SIZE][MAC_SIZE_BYTES];

  void clear()
  {
    count = 0;
  }

  void add(uint8_t mac[MAC_SIZE_BYTES])
  {
    if (count >= PEER_LIST_SIZE)
    {
      Serial.println("LOOKUP: Mac peers reached");
      return;
    }
    bool exists = false;
    for (size_t i = 0; i < count; i++)
    {
      if (memcmp(mac, set[i], MAC_SIZE_BYTES) == 0)
      {
        exists = true;
        break;
      }
    }

    if (!exists)
    {
      memcpy(set[count], mac, MAC_SIZE_BYTES);
      count++;
    }
  }
};

struct Mapping
{
  char topic[TOPIC_SIZE];
  MacSet macSet;
};

class TopicToMacsMap
{
public:
  size_t count = 0;
  Mapping map[TOPIC_LIST_SIZE * PEER_LIST_SIZE];

  void clear()
  {
    count = 0;
  }

  void init(EspNowData *espNowData)
  {
    clear();
    for (size_t i = 0; i < espNowData->peerCount; i++)
    {
      auto &peer = espNowData->peerList[i];
      for (size_t j = 0; j < peer.topicCount; j++)
      {
        auto &topic = peer.topicList[j];
        bool exists = false;
        for (size_t k = 0; k < count; k++)
        {
          if (strcmp(map[k].topic, topic) == 0)
          {
            exists = true;
            map[k].macSet.add(peer.mac);
            break;
          }
        }

        if (!exists)
        {
          Mapping mapping;
          strncpy(mapping.topic, topic, TOPIC_SIZE);
          mapping.macSet.add(peer.mac);
          memcpy(&map[count], &mapping, sizeof(Mapping));
          count++;
        }
      }
    }
  }

  Mapping *getMapping(const char *topic)
  {
    for (size_t i = 0; i < count; i++)
    {
      if (strcmp(topic, map[i].topic) == 0)
      {
        return &map[i];
      }
    }
    return nullptr;
  }
};

TopicSet topicSet;
TopicToMacsMap topicToMacsMap;