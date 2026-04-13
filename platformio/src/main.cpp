#include <time.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"

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
    // ── dispatch incoming commands here ─────────────────────────────
    String topic = String(event->topic, event->topic_len);
    String message = String(event->data, event->data_len);
    Serial.printf("MQTT: topic -> %s, data -> %s\n", topic, message);
    auto mapping = topicToMacsMap.getMapping(topic.c_str());
    if (mapping == nullptr)
      break;

    // prepare message
    EspNowMessage espNowMessage;

    for (size_t i = 0; i < mapping->macSet.count; i++)
    {
      auto &mac = mapping->macSet.set[i];
      // send to all subscribed devices
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

enum MessageType
{
  TEXT_MESSAGE = 1
};

struct MqttEspNowMessage
{
  uint16_t size;
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
  EspNowMessage incomingMsg;

  // Safety check to ensure we don't overflow our local struct
  int actualLen = (len > sizeof(EspNowMessage)) ? sizeof(EspNowMessage) : len;
  memcpy(&incomingMsg, data, actualLen);

  if (incomingMsg.type == MessageType::TEXT_MESSAGE)
  {
    Serial.println("------------------------------------------");
    Serial.printf("Data Length: %d\n", len);
    Serial.println("Message Content:");

    // Print the payload as a string
    Serial.println(incomingMsg.payload.mqttEspNowMessage.text);
    Serial.println("------------------------------------------\n");
  }
}

void onSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
  Serial.print("ESP-NOW: Send status -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void initEspNow()
{
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, deviceMac);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW: init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);
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
}

void loop()
{
  if (!ethernetConnected)
    return;

  timeClient.update(); // refresh NTP (cheap no-op if not due yet)

  // ── Periodic MQTT telemetry publish ───────────────────────────────────
  static uint32_t lastPublish = 0;
  if (!true)
  {
    lastPublish = millis();

    const char *txt = "Hello from gateway";

    EspNowMessage msg = {};
    msg.type = MessageType::TEXT_MESSAGE;
    msg.payload.mqttEspNowMessage.size = strlen(txt);
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