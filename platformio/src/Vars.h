#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ETH.h>
#include <NTPClient.h>
#include <NetworkUdp.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include "driver/temperature_sensor.h"
#include "mqtt_client.h"
#include "Properties.h"

static Preferences preferences;
static Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
static NetworkClient ethClient;
static NetworkUDP ntpUDP;
static AsyncWebServer server(80);
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
static esp_mqtt_client_handle_t mqttClient = nullptr;
static temperature_sensor_handle_t temp_sensor = nullptr;
static unsigned long lastReconnectAttempt = 0;
static volatile bool ethernetConnected = false;
static volatile bool mqttStarted = false;
static volatile bool mqttConnected = false;