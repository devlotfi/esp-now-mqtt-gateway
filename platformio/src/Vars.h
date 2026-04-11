#pragma once

#include <Arduino.h>
#include <ETH.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <NetworkUdp.h>
#include <ESP_SSLClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include "Properties.h"

Preferences preferences;
WebServer server(80);
NetworkClient ethClient;
ESP_SSLClient sslClient;
PubSubClient mqttClient(sslClient);
NetworkUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
bool ethernetConnected = false;
unsigned long lastReconnectAttempt = 0;