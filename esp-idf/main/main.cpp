#include <time.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
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
#include "SleepyInbox.h"
#include "HttpDispatcher.h"

void setupLookup()
{
  PeerData *peerData = loadPeerData();
  topicSet.init(peerData);
  topicToMacsMap.init(peerData);
}

void setup()
{
  Serial.begin(115200);
  esp_task_wdt_delete(NULL);

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
  preferences.begin(PREFERENCES_NAMESAPCE, false);
  Serial.println("NVS: Setup completed");

  // Lookup setup
  Serial.println("LOOKUP: Setup started");
  setupLookup();
  Serial.println("LOOKUP: Setup completed");

  Serial.println("SLEEPY-INBOX: Setup started");
  sleepyInbox.init();
  Serial.println("SLEEPY-INBOX: Setup completed");

  // esp-now setup
  Serial.println("ESP-NOW: Setup started");
  initEspNow();

  Serial.println("ESP-NOW: Init peers");
  initPeers();
  Serial.println("ESP-NOW: Init peers completed");

  Serial.println("ESP-NOW: Init sleepy peers");
  initSleepyPeers();
  Serial.println("ESP-NOW: Init sleepy peers completed");

  Serial.println("ESP-NOW: Init message queue");
  initQueue();
  Serial.println("ESP-NOW: Init message queue completed");

  Serial.println("ESP-NOW: Setup completed");

  // ethernet setup
  // Register Event Callback
  Serial.println("NETWORK-INTERFACE: Setup started");
  setupNetwork();
  Serial.println("NETWORK-INTERFACE: Setup completed");

  Serial.println("NTP-UPDATER: Setup started");
  initNtpUpdateTask();
  Serial.println("NTP-UPDATER: Setup completed");

  // http dispatcher task
  Serial.println("HTTP-DISPATCHER: Init task");
  initHttpDispatcherWorker();
  Serial.println("HTTP-DISPATCHER: Task init completed");

  // http server setup
  Serial.println("HTTP-SERVER: Setup started");
  setupServer();
  Serial.println("HTTP-SERVER: Setup completed");
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(10));
}