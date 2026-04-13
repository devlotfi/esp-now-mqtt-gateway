#include <time.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "EspNow.h"

void setupStorage()
{
  preferences.begin(PREFERENCES_NAMESAPCE, false);
}

void syncTime()
{
  Serial.println("Syncing time with NTP...");
  timeClient.begin();

  int retry = 0;
  while (!timeClient.update() && retry < 20)
  {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (retry < 20)
  {
    time_t epoch = timeClient.getEpochTime();
    struct timeval tv = {.tv_sec = epoch};
    settimeofday(&tv, nullptr);
    Serial.println("\nTime synced: " + timeClient.getFormattedTime());
  }
  else
  {
    Serial.println("\nNTP sync failed.");
  }
}

void setupLookup()
{
  EspNowData *espNowData = loadEspNowData();
  topicSet.init(espNowData);
  topicToMacsMap.init(espNowData);
  free(espNowData);
}

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

    size_t textLen = message.length();
    if (textLen >= ESP_NOW_TEXT_PAYLOAD_SIZE)
    {
      Serial.println("ESP-NOW: Message truncated (payload too large)");
      textLen = ESP_NOW_TEXT_PAYLOAD_SIZE - 1;
    }
    memcpy(mqttMsg.text, message.c_str(), textLen);
    // no null terminator needed — receiver derives length from frame size

    size_t sendSize = offsetof(EspNowMessage, payload) + offsetof(MqttEspNowMessage, text) + textLen; // <-- no +1 anymore

    for (size_t i = 0; i < mapping->macSet.count; i++)
    {
      char lol[MAC_SIZE_STRING];
      macBytesToString(mapping->macSet.set[i], lol);
      Serial.printf("Sending espnow message to %s\n", lol);
      Serial.printf("topic %s\n", espNowMessage.payload.mqttEspNowMessage.topic);
      Serial.printf("payload %s\n", espNowMessage.payload.mqttEspNowMessage.text);
      esp_err_t err = esp_now_send(mapping->macSet.set[i],
                                   (const uint8_t *)&espNowMessage, sendSize);
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

void WiFiEvent(arduino_event_id_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH: Started");
    ETH.setHostname("esp32-ethernet");
    break;

  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH: Link up");
    break;

  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH: IP → ");
    Serial.println(ETH.localIP());
    ethernetConnected = true;
    syncTime();
    if (!mqttStarted)
    {
      Serial.println("MQTT: Starting client...");
      startMqtt();
      mqttStarted = true;
    }
    break;

  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH: Link down");
    ethernetConnected = false;
    break;

  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH: Stopped");
    ethernetConnected = false;
    break;

  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  // NVS setup
  Serial.println("NVS: Setup started");
  setupStorage();
  Serial.println("NVS: Setup completed");

  // NVS setup
  Serial.println("LOOKUP: Setup started");
  setupLookup();
  Serial.println("LOOKUP: Setup completed");

  // esp-now setup
  Serial.println("ESP-NOW: Setup started");
  initEspNow();
  Serial.println("ESP-NOW: Init peers");
  initPeers();
  Serial.println("ESP-NOW: Init peers completed");
  Serial.println("ESP-NOW: Setup completed");

  // ethernet setup
  // Register Event Callback
  Serial.println("ETHERNET: Setup started");
  Network.onEvent(WiFiEvent);
  ETH.begin(ETH_PHY_W5500, 1, WIZNET_CS_PIN, WIZNET_INT_PIN, WIZNET_RESET_PIN, SPI2_HOST,
            WIZNET_SCLK_PIN, WIZNET_MISO_PIN, WIZNET_MOSI_PIN);
  Serial.println("ETHERNET: Setup completed");

  // http server setup
  Serial.println("HTTP-SERVER: Setup started");
  setupServer();
  Serial.println("HTTP-SERVER: Setup completed");

  /* uint8_t reciever[6] = {0x30, 0xAE, 0xA4, 0x11, 0x22, 0x23};
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, reciever, 6);
  peerInfo.encrypt = true;
  peerInfo.channel = 1;
  uint8_t pmk[16];
  keyHexToBytes("66536886DB697F700F780F3920F690A9", pmk);
  uint8_t lmk[16];
  keyHexToBytes("66536886DB697F700F780F3920F690A0", lmk);
  esp_now_set_pmk(pmk);
  memcpy(peerInfo.lmk, lmk, 16);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
  } */
}

void loop()
{
  if (!ethernetConnected)
    return;

  timeClient.update(); // refresh NTP (cheap no-op if not due yet)

  // ── Periodic MQTT telemetry publish ───────────────────────────────────
  static uint32_t lastPublish = 0;
  if (mqttStarted && (millis() - lastPublish >= 10000) && !true)
  {
    lastPublish = millis();

    const char *txt = "Hello from gateway";

    EspNowMessage msg = {};
    msg.type = MessageType::TEXT_MESSAGE;
    strcpy(msg.payload.mqttEspNowMessage.text, txt);
    // espnow
    uint8_t reciever[6] = {0x30, 0xAE, 0xA4, 0x11, 0x22, 0x23};

    auto result = esp_now_send(reciever, (uint8_t *)&msg, sizeof(msg));
    if (result == ESP_OK)
    {
      Serial.println("Packet accepted for transmission");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
      Serial.println("ESP-NOW not initialized");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
      Serial.println("Invalid argument (check MAC address)");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
      Serial.println("Internal error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
      Serial.println("ESP-NOW Out of Memory (Buffer full)");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
      Serial.println("Peer not found (did you call esp_now_add_peer?)");
    }

    Serial.println("ESP-NOW message sent");

    // Build a simple JSON payload
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"uptime_s\":%lu,\"heap_b\":%lu,\"time\":\"%s\"}",
             millis() / 1000,
             (unsigned long)ESP.getFreeHeap(),
             timeClient.getFormattedTime().c_str());

    int msgId = esp_mqtt_client_publish(
        mqttClient,
        "telemetry/test",
        payload,
        0, // len 0 = use strlen
        1, // QoS 1
        0  // not retained
    );

    if (msgId >= 0)
      Serial.printf("MQTT: Published → %s  (mid=%d)\n", payload, msgId);
    else
      Serial.println("MQTT: Publish failed (not connected yet?)");
  }
}