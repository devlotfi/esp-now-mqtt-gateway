#include <time.h>
#include <esp_now.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"

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

void setupTopics()
{
  EspNowData *espNowData = loadEspNowData();

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
    esp_mqtt_client_subscribe(mqttClient, "gateway/cmd", 1);
    break;

  case MQTT_EVENT_DISCONNECTED:
    Serial.println("MQTT: Disconnected (broker will retry automatically)");
    break;

  case MQTT_EVENT_DATA:
    Serial.printf("MQTT: [%.*s] → %.*s\n",
                  event->topic_len, event->topic,
                  event->data_len, event->data);
    // ── dispatch incoming commands here ─────────────────────────────
    break;

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

  // esp-now setup
  Serial.println("ESP-NOW: Setup started");
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW: init failed");
    return;
  }
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
  if (mqttStarted && (millis() - lastPublish >= 10000))
  {
    lastPublish = millis();

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