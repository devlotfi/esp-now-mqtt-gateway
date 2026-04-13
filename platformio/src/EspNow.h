#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"

enum MessageType
{
  TEXT_MESSAGE = 1
};

struct MqttEspNowMessage
{
  char topic[TOPIC_SIZE];
  char text[ESP_NOW_TEXT_PAYLOAD_SIZE];
};

struct EspNowMessage
{
  MessageType type;
  union
  {
    MqttEspNowMessage mqttEspNowMessage;
  } payload;
};

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
  if (!data || len < (int)sizeof(MessageType))
    return;

  const EspNowMessage *msg = reinterpret_cast<const EspNowMessage *>(data);

  switch (msg->type)
  {
  case MessageType::TEXT_MESSAGE:
  {
    constexpr int kHeaderSize = (int)(offsetof(EspNowMessage, payload) + offsetof(MqttEspNowMessage, text));
    if (len < kHeaderSize)
    {
      Serial.println("ESP-NOW: Frame too short, dropping");
      return;
    }

    const MqttEspNowMessage &mqttMsg = msg->payload.mqttEspNowMessage;

    char safeTopic[TOPIC_SIZE];
    memcpy(safeTopic, mqttMsg.topic, TOPIC_SIZE - 1);
    safeTopic[TOPIC_SIZE - 1] = '\0';

    int textLen = len - kHeaderSize;
    esp_mqtt_client_publish(mqttClient, safeTopic, mqttMsg.text, textLen, 1, 0);
    break;
  }

  default:
    Serial.printf("ESP-NOW: Unknown type %d, dropping\n", msg->type);
    break;
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