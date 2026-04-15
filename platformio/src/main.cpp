#include <time.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "Mqtt.h"
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
}

void loop()
{
  if (!ethernetConnected)
    return;
  timeClient.update();
}