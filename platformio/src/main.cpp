

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"

const char *mqtt_topic = "ethernet/test";

void setupStorage()
{
  preferences.begin(PREFERENCES_NAMESAPCE, false);
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  for (unsigned int i = 0; i < length; i++)
    Serial.print((char)payload[i]);

  Serial.println();
}

void connectMQTT()
{
  if (!ethernetConnected)
    return;

  if (mqttClient.connected())
    return;

  if (millis() - lastReconnectAttempt < 3000)
    return;

  lastReconnectAttempt = millis();

  Serial.print("Connecting MQTT... ");

  if (mqttClient.connect("ESP32_W5500_Client", mqtt_user, mqtt_password))
  {
    Serial.println("connected");
    mqttClient.subscribe(mqtt_topic);
  }
  else
  {
    Serial.print("failed rc=");
    Serial.println(mqttClient.state());
  }
}

/* =========================
   Core Functions
   ========================= */
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

void WiFiEvent(arduino_event_id_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH Started");
    // Set hostname if needed
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH Got IP: ");
    Serial.println(ETH.localIP());
    ethernetConnected = true;
    timeClient.begin();
    syncTime();
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    ethernetConnected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    ethernetConnected = false;
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  setupStorage();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Register Event Callback
  Network.onEvent(WiFiEvent);

  // Initialize W5500 via ETH.h
  // Params: PHY_TYPE, ADDR, CS, INT, RESET, SPI_HOST
  ETH.begin(ETH_PHY_W5500, 1, WIZNET_CS_PIN, WIZNET_INT_PIN, WIZNET_RESET_PIN, SPI2_HOST,
            WIZNET_SCLK_PIN, WIZNET_MISO_PIN, WIZNET_MOSI_PIN);

  // setupEndpoints();
  server.begin();
  Serial.println("HTTP server started");

  sslClient.setCACert(rootCA);
  sslClient.setBufferSizes(4096, 2048);
  sslClient.setClient(&ethClient);

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void loop()
{
  if (ethernetConnected)
  {
    server.handleClient();
    timeClient.update(); // Only update when we have a connection
  }

  connectMQTT();

  mqttClient.loop();

  static unsigned long lastMsg = 0;

  if (mqttClient.connected() && millis() - lastMsg > 30000)
  {
    lastMsg = millis();
    mqttClient.publish(mqtt_topic,
                       "Secure TLS message from ESP32 + W5500");
  }
}