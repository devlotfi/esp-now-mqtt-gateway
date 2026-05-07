#pragma once

#include <stdint.h>
#include "Properties.h"
#include "preferences/SleepyPeer.h"

struct SleepyPeerInbox
{
  bool isSet;
  uint8_t mac[MAC_SIZE_BYTES];
  char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
};

class SleepyInbox
{
public:
  static inline size_t sleepyInboxCount = 0;
  static inline SleepyPeerInbox *sleepyInbox = nullptr;

  static void init()
  {
    sleepyInboxCount = 0;
    sleepyInbox = (SleepyPeerInbox *)malloc(sizeof(SleepyPeerInbox) * SLEEPY_PEER_LIST_SIZE);

    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();
    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      sleepyInbox[sleepyInboxCount].isSet = false;
      memcpy(sleepyInbox[sleepyInboxCount].mac, sleepyPeer.mac, MAC_SIZE_BYTES);
      sleepyInboxCount++;
    }
  }

  static bool set(uint8_t mac[MAC_SIZE_BYTES], const char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE])
  {
    for (size_t i = 0; i < sleepyInboxCount; i++)
    {
      auto &sleepyPeerInbox = sleepyInbox[i];
      if (memcmp(sleepyPeerInbox.mac, mac, MAC_SIZE_BYTES) == 0)
      {
        sleepyPeerInbox.isSet = true;
        memcpy(sleepyPeerInbox.text, text, MQTT_MESSAGE_TEXT_PAYLOAD_SIZE);
        return true;
      }
    }
    return false;
  }

  static bool get(char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE], uint8_t mac[MAC_SIZE_BYTES])
  {
    for (size_t i = 0; i < sleepyInboxCount; i++)
    {
      auto &sleepyPeerInbox = sleepyInbox[i];
      if (memcmp(sleepyPeerInbox.mac, mac, MAC_SIZE_BYTES) == 0 && sleepyPeerInbox.isSet)
      {
        memcpy(text, sleepyPeerInbox.text, MQTT_MESSAGE_TEXT_PAYLOAD_SIZE);
        return true;
      }
    }
    return false;
  }

  static bool clear(uint8_t mac[MAC_SIZE_BYTES])
  {
    for (size_t i = 0; i < sleepyInboxCount; i++)
    {
      auto &sleepyPeerInbox = sleepyInbox[i];
      if (memcmp(sleepyPeerInbox.mac, mac, MAC_SIZE_BYTES) == 0)
      {
        sleepyPeerInbox.isSet = false;
        return true;
      }
    }
    return false;
  }
};

static SleepyInbox sleepyInbox;