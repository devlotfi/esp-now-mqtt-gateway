#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "Notifications.h"
#include "Grafana.h"
#include "preferences/Notifications.h"
#include "preferences/EspNow.h"
#include "preferences/Peer.h"
#include "preferences/SleepyPeer.h"
#include "EspNowMessageQueue.h"
#include "SleepyInbox.h"
#include "Wol.h"

enum MessageType
{
  TEXT_MESSAGE = 1,
  NOTIFICATION_MESSAGE = 2,
  TIME_SYNC_MESSAGE = 3,
  SLEEPY_COMMAND_MESSAGE = 4,
  SLEEPY_DATA_MESSAGE = 5,
  WOL_MESSAGE = 6,
  METRIC_MESSAGE = 7
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
  char timezonePosix[64];
};

struct __attribute__((packed)) SleepyCommandEspNowMessage
{
  char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
};

struct __attribute__((packed)) SleepyDataEspNowMessage
{
  char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
};

struct __attribute__((packed)) WolEspNowMessage
{
  uint16_t port;
  uint8_t mac[MAC_SIZE_BYTES];
};

struct __attribute__((packed)) MetricEspNowMessage
{
  char body[GRAFANA_BODY_SIZE];
};

struct __attribute__((packed)) EspNowMessage
{
  MessageType type;
  union
  {
    MqttEspNowMessage mqttEspNowMessage;
    NotificationEspNowMessage notificationEspNowMessage;
    TimeSyncEspNowMessage timeSyncEspNowMessage;
    SleepyCommandEspNowMessage sleepyCommandEspNowMessage;
    SleepyDataEspNowMessage sleepyDataEspNowMessage;
    WolEspNowMessage wolEspNowMessage;
    MetricEspNowMessage metricEspNowMessage;
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
    break;
  }
  case MessageType::TIME_SYNC_MESSAGE:
  {
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
    strlcpy(timeSyncMsg.timezonePosix, TIMEZONE_POSIX, TIMEZONE_POSIX_SIZE);

    printCurrentTime();

    bool res = enqueueMessage(info->src_addr,
                              (const uint8_t *)&espNowMessage,
                              sizeof(EspNowMessage));
    if (!res)
      Serial.println("ESP-NOW: Send failed");
    break;
  }
  case MessageType::SLEEPY_DATA_MESSAGE:
  {
    const SleepyDataEspNowMessage &sleepyDataMsg = msg->payload.sleepyDataEspNowMessage;
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();

    SleepyPeer *sourceSleepyPeer = nullptr;
    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      if (memcmp(sleepyPeer.mac, info->src_addr, MAC_SIZE_BYTES) == 0)
      {
        sourceSleepyPeer = &sleepyPeer;
      }
    }
    if (!sourceSleepyPeer)
      return;

    esp_mqtt_client_publish(
        mqttClient,
        sourceSleepyPeer->dataTopic,
        sleepyDataMsg.text,
        0,
        1,
        1);
    break;
  }
  case MessageType::SLEEPY_COMMAND_MESSAGE:
  {
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();

    SleepyPeer *sourceSleepyPeer = nullptr;
    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      if (memcmp(sleepyPeer.mac, info->src_addr, MAC_SIZE_BYTES) == 0)
      {
        sourceSleepyPeer = &sleepyPeer;
      }
    }
    if (!sourceSleepyPeer)
      return;

    char text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
    bool hasCommand = sleepyInbox.get(text, info->src_addr);
    if (!hasCommand)
      return;

    // Build the ESP-NOW frame
    Serial.println("ESP-NOW: Sleepy command sent");
    EspNowMessage espNowMessage = {};
    espNowMessage.type = MessageType::SLEEPY_COMMAND_MESSAGE;
    SleepyCommandEspNowMessage &sleepyCommandMsg = espNowMessage.payload.sleepyCommandEspNowMessage;

    strlcpy(sleepyCommandMsg.text, text, MQTT_MESSAGE_TEXT_PAYLOAD_SIZE - 1);

    bool res = enqueueMessage(
        sourceSleepyPeer->mac,
        (const uint8_t *)&espNowMessage,
        sizeof(EspNowMessage));
    if (!res)
      Serial.println("ESP-NOW: Send failed");
    sleepyInbox.clear(sourceSleepyPeer->mac);
    break;
  }
  case MessageType::WOL_MESSAGE:
  {
    const WolEspNowMessage &wolMsg = msg->payload.wolEspNowMessage;
    sendWakeOnLan(wolMsg.port, wolMsg.mac);
    break;
  }
  case MessageType::METRIC_MESSAGE:
  {
    const MetricEspNowMessage &metricMsg = msg->payload.metricEspNowMessage;
    GrafanaData *grafanaData = loadGrafanaData();
    if (!grafanaData->isSet)
    {
      Serial.println("ESP-NOW: Grafana data not set");
      return;
    }

    saveGrafanaMetric(grafanaData, metricMsg.body);
    break;
  }
  default:
  {
    Serial.printf("ESP-NOW: Unknown type %d\n", msg->type);
    break;
  }
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
}

void initPeers()
{
  EspNowData *espNowData = loadEspNowData();
  PeerData *peerData = loadPeerData();
  for (size_t i = 0; i < peerData->peerCount; i++)
  {
    auto &peer = peerData->peerList[i];
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = espNowData->channel;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, peer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, peer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("ESP-NOW: Cannot add peer");
      return;
    }
  }
}

void initSleepyPeers()
{
  EspNowData *espNowData = loadEspNowData();
  SleepyPeerData *sleepyPeerData = loadSleepyPeerData();
  for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
  {
    auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
    esp_now_peer_info_t peerInfo{};
    peerInfo.channel = espNowData->channel;
    peerInfo.encrypt = true;
    memcpy(peerInfo.peer_addr, sleepyPeer.mac, MAC_SIZE_BYTES);
    memcpy(peerInfo.lmk, sleepyPeer.lmk, ESP_NOW_KEY_SIZE_BYTES);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("ESP-NOW: Cannot add peer");
      return;
    }
  }
}

void clearPeers()
{
  PeerData *peerData = loadPeerData();
  for (size_t i = 0; i < peerData->peerCount; i++)
  {
    auto &peer = peerData->peerList[i];
    esp_now_del_peer(peer.mac);
  }
}

void clearSleepyPeers()
{
  SleepyPeerData *sleepyPeerData = loadSleepyPeerData();
  for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
  {
    auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
    esp_now_del_peer(sleepyPeer.mac);
  }
}