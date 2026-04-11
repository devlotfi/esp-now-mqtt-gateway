#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ETH.h>
#include <NTPClient.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include "Properties.h"

Preferences preferences;
NetworkClient ethClient;
NetworkUDP ntpUDP;
AsyncWebServer server(80);
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
bool ethernetConnected = false;
unsigned long lastReconnectAttempt = 0;