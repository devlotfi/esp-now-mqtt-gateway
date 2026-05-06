#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "mqtt_client.h"
#include "Properties.h"

static Preferences preferences;
static esp_mqtt_client_handle_t mqttClient = nullptr;
static unsigned long lastReconnectAttempt = 0;
static volatile bool ethernetConnected = false;
static volatile bool mqttStarted = false;
static volatile bool mqttConnected = false;
static volatile bool ntpSynced = false;