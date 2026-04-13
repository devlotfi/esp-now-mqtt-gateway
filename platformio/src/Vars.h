#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ETH.h>
#include <NTPClient.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include <etl/set.h>
#include <etl/vector.h>
#include <etl/map.h>
#include <etl/string.h>
#include "mqtt_client.h"
#include "Properties.h"

Preferences preferences;
NetworkClient ethClient;
NetworkUDP ntpUDP;
AsyncWebServer server(80);
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
esp_mqtt_client_handle_t mqttClient = nullptr;
bool ethernetConnected = false;
unsigned long lastReconnectAttempt = 0;
bool mqttStarted = false;