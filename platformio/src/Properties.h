#pragma once

#include <Arduino.h>

// ----------------------
// Data config
// ----------------------
#define API_BUFFER 8192

#define TOKEN_PAYLOAD_SIZE 16
#define TOKEN_SIGNATURE_SIZE 32

#define PASSWORD_SALT_SIZE 16
#define PASSWORD_HASH_SIZE 32

#define NAME_SIZE 32
#define UUID_V4_SIZE 37
#define MAC_SIZE_BYTES 6
#define MAC_SIZE_STRING 18
#define ESP_NOW_KEY_SIZE_BYTES 16
#define ESP_NOW_KEY_SIZE_STRING 33
#define PEER_LIST_SIZE 10

#define TOPIC_LIST_SIZE 10
#define TOPIC_SIZE 48

#define MQTT_MESSAGE_TEXT_PAYLOAD_SIZE 1300

#define NOTIFICATION_TITLE_SIZE 128
#define NOTIFICATION_BODY_SIZE 1200
#define NOTIFICATIONS_API_URL_SIZE 256
#define NOTIFICATIONS_API_SECRET_SIZE 256

// ----------------------
// Led
// ----------------------
#define LED_PIN 48
#define LED_COUNT 1

// ----------------------
// Preferences namespace
// ----------------------
#define PREFERENCES_NAMESAPCE "DEVICE"

// ----------------------
// W5500 Pin Configuration
// ----------------------
#define WIZNET_CS_PIN 10
#define WIZNET_RESET_PIN 9
#define WIZNET_MISO_PIN 13
#define WIZNET_MOSI_PIN 11
#define WIZNET_SCLK_PIN 12
#define WIZNET_INT_PIN 15

// ----------------------
// MQTT
// ----------------------
#define MQTT_CLIENT_ID_SIZE 128
#define MQTT_USERNAME_SIZE 128
#define MQTT_PASSWORD_SIZE 128
#define MQTT_URL_SIZE 512
const char *mqtt_uri = "mqtts://x9cb5c45.ala.eu-central-1.emqxsl.com:8883";
const char *mqtt_user = "test";
const char *mqtt_password = "test";

// ----------------------
// ESP-NOW
// ----------------------
uint8_t defaultMac[6] = {0x30, 0xAE, 0xA4, 0x11, 0x22, 0x33};

// ----------------------
// JWT Secret
// ----------------------
const char *jwt_secret = "super_secret_device_key_123";