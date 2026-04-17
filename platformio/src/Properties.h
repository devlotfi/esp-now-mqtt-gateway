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
#define PEER_LIST_SIZE 10

#define TOPIC_LIST_SIZE 10
#define TOPIC_SIZE 48

#define MQTT_MESSAGE_TEXT_PAYLOAD_SIZE 1300

#define NOTIFICATION_TITLE_SIZE 128
#define NOTIFICATION_BODY_SIZE 1200
#define NOTIFICATIONS_API_URL_SIZE 256
#define NOTIFICATIONS_API_SECRET_SIZE 256

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
// MQTT credentials
// ----------------------
const char *mqtt_uri = "mqtts://x9cb5c45.ala.eu-central-1.emqxsl.com:8883";
const char *mqtt_user = "test";
const char *mqtt_password = "test";

// ----------------------
// ESP-NOW
// ----------------------
uint8_t deviceMac[6] = {0x30, 0xAE, 0xA4, 0x11, 0x22, 0x33};

// ----------------------
// JWT Secret
// ----------------------
const char *jwt_secret = "super_secret_device_key_123";

// ----------------------
// TLS Root Certificate
// ----------------------
const char rootCA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";