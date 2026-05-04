#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct MqttData
{
  bool isSet;
  bool useAuth;
  char clientId[MQTT_CLIENT_ID_SIZE];
  char username[MQTT_USERNAME_SIZE];
  char password[MQTT_PASSWORD_SIZE];
  char url[MQTT_URL_SIZE];
  bool useSleepyPeerDiscovery;
  char discoveryRequestTopic[TOPIC_SIZE];
  char discoveryResponseTopic[TOPIC_SIZE];
};

static MqttData *mqttDataCache = nullptr;

void saveMqttData(const MqttData *data)
{
  size_t written = preferences.putBytes("mqtt_data", data, sizeof(MqttData));
  if (written != sizeof(MqttData))
    return;
  if (!mqttDataCache)
    mqttDataCache = (MqttData *)malloc(sizeof(MqttData));
  if (mqttDataCache)
    memcpy(mqttDataCache, data, sizeof(MqttData));
}

MqttData *loadMqttData()
{
  if (mqttDataCache)
    return mqttDataCache;

  mqttDataCache = (MqttData *)malloc(sizeof(MqttData));
  if (!mqttDataCache)
    return nullptr;
  mqttDataCache->isSet = false;
  mqttDataCache->useAuth = false;
  mqttDataCache->useSleepyPeerDiscovery = false;
  preferences.getBytes("mqtt_data", mqttDataCache, sizeof(MqttData));
  return mqttDataCache;
}
