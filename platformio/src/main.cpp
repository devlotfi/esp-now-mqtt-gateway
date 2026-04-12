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
  if (ethernetConnected)
  {
    timeClient.update(); // Only update when we have a connection
  }
}