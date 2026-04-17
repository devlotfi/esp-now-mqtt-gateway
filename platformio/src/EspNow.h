#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"

enum MessageType
{
  TEXT_MESSAGE = 1,
  NOTIFICATION_MESSAGE = 2
};

struct __attribute__((packed)) MqttEspNowMessage
{
  char topic[TOPIC_SIZE];
  char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
};

struct __attribute__((packed)) NotificationEspNowMessage
{
  char title[NOTIFICATION_TITLE_SIZE];
  char body[NOTIFICATION_BODY_SIZE];
};

struct __attribute__((packed)) EspNowMessage
{
  MessageType type;
  union
  {
    MqttEspNowMessage mqttEspNowMessage;
    NotificationEspNowMessage notificationEspNowMessage;
  } payload;
};

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
  if (!data || len != sizeof(EspNowMessage))
  {
    Serial.println("ESP-NOW: Invalid frame size");
    return;
  }

  const EspNowMessage *msg = reinterpret_cast<const EspNowMessage *>(data);

  switch (msg->type)
  {
  case TEXT_MESSAGE:
  {
    const MqttEspNowMessage &mqttMsg = msg->payload.mqttEspNowMessage;

    char safeTopic[TOPIC_SIZE];
    memcpy(safeTopic, mqttMsg.topic, TOPIC_SIZE - 1);
    safeTopic[TOPIC_SIZE - 1] = '\0';

    esp_mqtt_client_publish(
        mqttClient,
        safeTopic,
        mqttMsg.text,
        0,
        1,
        0);

    break;
  }

  default:
    Serial.printf("ESP-NOW: Unknown type %d\n", msg->type);
  }
}

void onSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
  Serial.print("ESP-NOW: Send status -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void initEspNow()
{
  EspNowData *espNowData = loadEspNowData();
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, deviceMac);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_ps(WIFI_PS_NONE);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW: init failed");
    return;
  }
  if (espNowData->pmkSet)
  {
    Serial.println("ESP-NOW: Settings PMK");
    esp_now_set_pmk(espNowData->pmk);
  }

  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);
  free(espNowData);
}

void initPeers()
{
  EspNowData *espNowData = loadEspNowData();
  for (size_t i = 0; i < espNowData->peerCount; i++)
  {
    auto &peer = espNowData->peerList[i];
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = 1;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, peer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, peer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      free(espNowData);
      Serial.println("ESP-NOW: Cannot add peer");
      return;
    }
  }
  free(espNowData);
}

void clearPeers()
{
  EspNowData *espNowData = loadEspNowData();
  for (size_t i = 0; i < espNowData->peerCount; i++)
  {
    auto &peer = espNowData->peerList[i];
    esp_now_del_peer(peer.mac);
  }
  free(espNowData);
}