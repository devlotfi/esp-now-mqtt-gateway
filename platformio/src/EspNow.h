#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "Notifications.h"
#include "preferences/Notifications.h"

enum MessageType
{
  TEXT_MESSAGE = 1,
  NOTIFICATION_MESSAGE = 2,
  TIME_SYNC_MESSAGE = 3
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

struct __attribute__((packed)) TimeSyncEspNowMessage
{
  uint32_t epoch;
};

struct __attribute__((packed)) EspNowMessage
{
  MessageType type;
  union
  {
    MqttEspNowMessage mqttEspNowMessage;
    NotificationEspNowMessage notificationEspNowMessage;
    TimeSyncEspNowMessage timeSyncEspNowMessage;
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
  case MessageType::TEXT_MESSAGE:
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
  case MessageType::NOTIFICATION_MESSAGE:
  {
    const NotificationEspNowMessage &notificationMsg = msg->payload.notificationEspNowMessage;
    NotificationsData *notificationsData = loadNotificationsData();
    if (!notificationsData->isSet)
    {
      Serial.println("ESP-NOW: Notification data not set");
      return;
    }

    sendNotification(notificationsData, notificationMsg.title, notificationMsg.body);
    free(notificationsData);
    break;
  }
  case MessageType::TIME_SYNC_MESSAGE:
  {
    // Build the ESP-NOW frame
    Serial.println("ESP-NOW: Time Sync Request");
    EspNowMessage espNowMessage = {};
    espNowMessage.type = MessageType::TIME_SYNC_MESSAGE;

    TimeSyncEspNowMessage &timeSyncMsg = espNowMessage.payload.timeSyncEspNowMessage;

    time_t now;
    time(&now);
    if (now < 1577836800)
    {
      Serial.println("ESP-NOW: Time not synced, canceling time sync");
      return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    timeSyncMsg.epoch = tv.tv_sec;

    esp_err_t err = esp_now_send(
        info->src_addr,
        (const uint8_t *)&espNowMessage,
        sizeof(EspNowMessage));

    if (err != ESP_OK)
      Serial.printf("ESP-NOW: Send failed, err=0x%x\n", err);
    break;
  }
  default:
    Serial.printf("ESP-NOW: Unknown type %d\n", msg->type);
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

  char macStr[MAC_SIZE_STRING];
  macBytesToString(espNowData->mac, macStr);
  Serial.printf("ESP-NOW: Using mac: %s\n", macStr);
  esp_wifi_set_mac(WIFI_IF_STA, espNowData->mac);

  esp_wifi_set_channel(espNowData->channel, WIFI_SECOND_CHAN_NONE);
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