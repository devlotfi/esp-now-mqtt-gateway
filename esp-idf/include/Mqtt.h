#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "Led.h"
#include "preferences/Mqtt.h"
#include "preferences/SleepyPeer.h"
#include "EspNowMessageQueue.h"
#include "SleepyInbox.h"

static void mqttEventHandler(void *args, esp_event_base_t base,
                             int32_t eventId, void *eventData)
{
  auto *event = static_cast<esp_mqtt_event_handle_t>(eventData);

  switch (static_cast<esp_mqtt_event_id_t>(eventId))
  {
  case MQTT_EVENT_CONNECTED:
  {
    mqttConnected = true;
    updateLed();
    Serial.println("MQTT: Connected");

    MqttData *mqttData = loadMqttData();
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();

    // subscribe to sleepy peer discovery
    if (mqttData->useSleepyPeerDiscovery)
    {
      esp_mqtt_client_subscribe(mqttClient, mqttData->discoveryRequestTopic, 1);
      Serial.print("MQTT: Subscribing to sleepy peer discovery -> ");
      Serial.print(mqttData->discoveryRequestTopic);
      Serial.println();
    }

    // subscribe to sleepy peer command topic
    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      esp_mqtt_client_subscribe(mqttClient, sleepyPeer.commandTopic, 1);
      Serial.print("MQTT: Subscribing to sleepy peer command topic discovery -> ");
      Serial.print(sleepyPeer.commandTopic);
    }

    // subscribe to peer topics
    for (size_t i = 0; i < topicSet.count; i++)
    {
      esp_mqtt_client_subscribe(mqttClient, topicSet.set[i], 1);
      Serial.print("MQTT: Subscribing to -> ");
      Serial.print(topicSet.set[i]);
      Serial.println();
    }
    break;
  }
  case MQTT_EVENT_DISCONNECTED:
  {
    mqttConnected = false;
    updateLed();
    Serial.println("MQTT: Disconnected (broker will retry automatically)");
    break;
  }
  case MQTT_EVENT_DATA:
  {
    if (event->current_data_offset != 0 || event->data_len != event->total_data_len)
    {
      Serial.println("MQTT: Chunked message not supported, dropping");
      break;
    }

    // Bound-check against our fixed buffers instead of using String (which
    // would allocate/free from internal heap on every single message).
    if (event->topic_len >= TOPIC_SIZE)
    {
      Serial.println("MQTT: Topic too long, dropping");
      break;
    }
    if (event->data_len >= MQTT_MESSAGE_TEXT_PAYLOAD_SIZE)
    {
      Serial.println("MQTT: Payload too long, dropping");
      break;
    }

    char topic[TOPIC_SIZE];
    memcpy(topic, event->topic, event->topic_len);
    topic[event->topic_len] = '\0';

    char message[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE];
    memcpy(message, event->data, event->data_len);
    message[event->data_len] = '\0';

    Serial.printf("MQTT: topic -> %s, data -> %s\n", topic, message);

    MqttData *mqttData = loadMqttData();
    SleepyPeerData *sleepyPeerData = loadSleepyPeerData();

    if (mqttData->isSet && mqttData->useSleepyPeerDiscovery && strncmp(mqttData->discoveryRequestTopic, topic, TOPIC_SIZE) == 0)
    {
      for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
      {
        auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];

        ArduinoJson::JsonDocument doc;
        doc["id"] = sleepyPeer.id;
        doc["name"] = sleepyPeer.name;
        doc["commandTopic"] = sleepyPeer.commandTopic;
        doc["dataTopic"] = sleepyPeer.dataTopic;

        char jsonBuffer[1024];
        ArduinoJson::serializeJson(doc, jsonBuffer, 1024);
        jsonBuffer[1024 - 1] = '\0';

        esp_mqtt_client_publish(
            mqttClient,
            mqttData->discoveryResponseTopic,
            jsonBuffer,
            0,
            1,
            0);
      }
      return;
    }

    for (size_t i = 0; i < sleepyPeerData->sleepyPeerCount; i++)
    {
      auto &sleepyPeer = sleepyPeerData->sleepyPeerList[i];
      if (strncmp(sleepyPeer.commandTopic, topic, TOPIC_SIZE) == 0)
      {
        sleepyInbox.set(sleepyPeer.mac, message);
        Serial.println("MQTT: Saved in sleepy inbox");
      }
    }

    auto mapping = topicToMacsMap.getMapping(topic);
    if (mapping == nullptr)
      break;

    // Build the ESP-NOW frame
    EspNowMessage espNowMessage = {};
    espNowMessage.type = MessageType::TEXT_MESSAGE;

    MqttEspNowMessage &mqttMsg = espNowMessage.payload.mqttEspNowMessage;

    strlcpy(mqttMsg.topic, topic, TOPIC_SIZE - 1);
    strlcpy(mqttMsg.text, message, MQTT_MESSAGE_TEXT_PAYLOAD_SIZE - 1);

    for (size_t i = 0; i < mapping->macSet.count; i++)
    {
      bool res = enqueueMessage(
          mapping->macSet.set[i],
          (const uint8_t *)&espNowMessage,
          sizeof(EspNowMessage));
      if (!res)
        Serial.println("ESP-NOW: Send failed");
    }
    break;
  }
  case MQTT_EVENT_ERROR:
  {
    Serial.println("MQTT: Error");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      Serial.printf("  esp-tls : 0x%x\n", event->error_handle->esp_tls_last_esp_err);
      Serial.printf("  mbedTLS : 0x%x\n", event->error_handle->esp_tls_stack_err);
      Serial.printf("  errno   : %d\n", event->error_handle->esp_transport_sock_errno);
    }
    break;
  }
  default:
    break;
  }
}

// Tear down any previously running MQTT client instance cleanly before
// creating a new one. Safe to call even if mqttClient is nullptr.
static void stopMqtt()
{
  if (mqttClient != nullptr)
  {
    Serial.println("MQTT: Stopping existing client...");
    esp_mqtt_client_stop(mqttClient);
    esp_mqtt_client_destroy(mqttClient);
    mqttClient = nullptr;
    mqttConnected = false;
    Serial.println("MQTT: Client destroyed");
  }
}

static void startMqtt()
{
  MqttData *mqttData = loadMqttData();

  if (!mqttData->isSet)
  {
    Serial.println("MQTT: No configuration saved, skipping");
    return;
  }

  stopMqtt();

  static char url[MQTT_URL_SIZE];
  static char clientId[MQTT_CLIENT_ID_SIZE];
  static char username[MQTT_USERNAME_SIZE];
  static char password[MQTT_PASSWORD_SIZE];

  strlcpy(url, mqttData->url, MQTT_URL_SIZE);
  strlcpy(clientId, mqttData->clientId, MQTT_CLIENT_ID_SIZE);
  strlcpy(username, mqttData->username, MQTT_USERNAME_SIZE);
  strlcpy(password, mqttData->password, MQTT_PASSWORD_SIZE);

  esp_mqtt_client_config_t cfg = {};
  cfg.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
  cfg.broker.address.uri = url;
  cfg.credentials.client_id = clientId;
  cfg.credentials.username = username;
  cfg.credentials.authentication.password = password;

  mqttClient = esp_mqtt_client_init(&cfg);
  esp_mqtt_client_register_event(mqttClient,
                                 (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 mqttEventHandler, nullptr);
  esp_mqtt_client_start(mqttClient);
  Serial.println("MQTT: Client started");
}