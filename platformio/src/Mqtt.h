#pragma once

#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"

static void mqttEventHandler(void *args, esp_event_base_t base,
                             int32_t eventId, void *eventData)
{
  auto *event = static_cast<esp_mqtt_event_handle_t>(eventData);

  switch (static_cast<esp_mqtt_event_id_t>(eventId))
  {
  case MQTT_EVENT_CONNECTED:
    Serial.println("MQTT: Connected");
    for (size_t i = 0; i < topicSet.count; i++)
    {
      esp_mqtt_client_subscribe(mqttClient, topicSet.set[i], 1);
      Serial.print("MQTT: Subscribing to -> ");
      Serial.print(topicSet.set[i]);
      Serial.println();
    }
    break;

  case MQTT_EVENT_DISCONNECTED:
    Serial.println("MQTT: Disconnected (broker will retry automatically)");
    break;

  case MQTT_EVENT_DATA:
  {
    if (event->current_data_offset != 0 || event->data_len != event->total_data_len)
    {
      Serial.println("MQTT: Chunked message not supported, dropping");
      break;
    }

    String topic = String(event->topic, event->topic_len);
    String message = String(event->data, event->data_len);
    Serial.printf("MQTT: topic -> %s, data -> %s\n", topic.c_str(), message.c_str());

    auto mapping = topicToMacsMap.getMapping(topic.c_str());
    if (mapping == nullptr)
      break;

    // Build the ESP-NOW frame
    EspNowMessage espNowMessage = {};
    espNowMessage.type = MessageType::TEXT_MESSAGE;

    MqttEspNowMessage &mqttMsg = espNowMessage.payload.mqttEspNowMessage;

    strncpy(mqttMsg.topic, topic.c_str(), TOPIC_SIZE - 1);
    mqttMsg.topic[TOPIC_SIZE - 1] = '\0';

    strncpy(mqttMsg.text, message.c_str(), MQTT_MESSAGE_TEXT_PAYLOAD_SIZE - 1);
    mqttMsg.text[MQTT_MESSAGE_TEXT_PAYLOAD_SIZE - 1] = '\0';

    for (size_t i = 0; i < mapping->macSet.count; i++)
    {
      esp_err_t err = esp_now_send(
          mapping->macSet.set[i],
          (const uint8_t *)&espNowMessage,
          sizeof(EspNowMessage));

      if (err != ESP_OK)
        Serial.printf("ESP-NOW: Send failed, err=0x%x\n", err);
    }
    break;
  }

  case MQTT_EVENT_ERROR:
    Serial.println("MQTT: Error");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      Serial.printf("  esp-tls : 0x%x\n", event->error_handle->esp_tls_last_esp_err);
      Serial.printf("  mbedTLS : 0x%x\n", event->error_handle->esp_tls_stack_err);
      Serial.printf("  errno   : %d\n", event->error_handle->esp_transport_sock_errno);
    }
    break;

  default:
    break;
  }
}

static void startMqtt()
{
  esp_mqtt_client_config_t cfg = {};
  cfg.broker.address.uri = mqtt_uri;
  cfg.broker.verification.certificate = rootCA;
  cfg.credentials.client_id = "test-client";
  cfg.credentials.username = mqtt_user;
  cfg.credentials.authentication.password = mqtt_password;

  mqttClient = esp_mqtt_client_init(&cfg);
  esp_mqtt_client_register_event(mqttClient,
                                 (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 mqttEventHandler, nullptr);
  esp_mqtt_client_start(mqttClient);
}