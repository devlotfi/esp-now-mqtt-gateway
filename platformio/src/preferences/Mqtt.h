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
};

void saveMqttData(const MqttData *data)
{
  preferences.putBytes("mqtt_data", data, sizeof(MqttData));
}

MqttData *loadMqttData()
{
  MqttData *data = (MqttData *)malloc(sizeof(MqttData));
  if (!data)
    return nullptr;
  data->isSet = false;
  data->useAuth = false;
  preferences.getBytes("mqtt_data", data, sizeof(MqttData));
  return data;
}
