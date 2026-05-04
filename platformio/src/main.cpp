#include <time.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "Properties.h"
#include "Vars.h"
#include "Api.h"
#include "Lookup.h"
#include "Mqtt.h"
#include "EspNow.h"
#include "TempSensor.h"
#include "NetworkSetup.h"
#include "Led.h"
#include "Watchdog.h"
#include "EspNowMessageQueue.h"

void setupStorage()
{
  preferences.begin(PREFERENCES_NAMESAPCE, false);
}

void setupLookup()
{
  PeerData *peerData = loadPeerData();
  topicSet.init(peerData);
  topicToMacsMap.init(peerData);
}

void setup()
{
  Serial.begin(115200);

  // Watchdog setup
  Serial.println("WATCHDOG: Setup started");
  setupWatchdog();
  Serial.println("WATCHDOG: Setup completed");

  // LED setup
  Serial.println("LED: Setup started");
  setupLed();
  Serial.println("LED: Setup completed");

  // NVS setup
  Serial.println("TEMP-SENSOR: Setup started");
  setupTempSensor();
  Serial.println("TEMP-SENSOR: Setup completed");

  // NVS setup
  Serial.println("NVS: Setup started");
  setupStorage();
  Serial.println("NVS: Setup completed");

  // Lookup setup
  Serial.println("LOOKUP: Setup started");
  setupLookup();
  Serial.println("LOOKUP: Setup completed");

  // esp-now setup
  Serial.println("ESP-NOW: Setup started");
  initEspNow();

  Serial.println("ESP-NOW: Init peers");
  initPeers();
  Serial.println("ESP-NOW: Init peers completed");

  Serial.println("ESP-NOW: Init message queue");
  initQueue();
  Serial.println("ESP-NOW: Init message queue completed");

  Serial.println("ESP-NOW: Setup completed");

  // ethernet setup
  // Register Event Callback
  Serial.println("NETWORK-INTERFACE: Setup started");
  setupNetwork();
  Serial.println("NETWORK-INTERFACE: Setup completed");

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