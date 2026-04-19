#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ETH.h>
#include <NTPClient.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include "driver/temperature_sensor.h"
#include "mqtt_client.h"
#include "Properties.h"

static Preferences preferences;
static NetworkClient ethClient;
static NetworkUDP ntpUDP;
static AsyncWebServer server(80);
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
static esp_mqtt_client_handle_t mqttClient = nullptr;
static temperature_sensor_handle_t temp_sensor = nullptr;
static bool ethernetConnected = false;
static unsigned long lastReconnectAttempt = 0;
static bool mqttStarted = false;
static volatile bool mqttConnected = false;