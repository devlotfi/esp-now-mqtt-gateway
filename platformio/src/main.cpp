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
  setupServer();
  Serial.println("HTTP server ready");
}

void loop()
{
  if (ethernetConnected)
  {
    timeClient.update(); // Only update when we have a connection
  }
}