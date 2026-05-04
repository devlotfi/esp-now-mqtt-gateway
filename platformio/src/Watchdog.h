#pragma once

#include "Vars.h"
#include "preferences/Network.h"

#define WATCHDOG_CHECK_INTERVAL_MS (30UL * 1000UL)       // check every 30s
#define WATCHDOG_ETH_RESET_MS (1UL * 60UL * 1000UL)      // reset ETH after 1min
#define WATCHDOG_REBOOT_TIMEOUT_MS (3UL * 60UL * 1000UL) // reboot after 5min

static void resetEthernet()
{
  Serial.println("WATCHDOG: Resetting W5500...");
  pinMode(WIZNET_RESET_PIN, OUTPUT);
  digitalWrite(WIZNET_RESET_PIN, LOW);
  delay(200);
  digitalWrite(WIZNET_RESET_PIN, HIGH);
  delay(500);

  NetworkData *networkData = loadNetworkData();
  ETH.begin(ETH_PHY_W5500, 1, WIZNET_CS_PIN, WIZNET_INT_PIN, WIZNET_RESET_PIN, SPI2_HOST,
            WIZNET_SCLK_PIN, WIZNET_MISO_PIN, WIZNET_MOSI_PIN);
  if (networkData->ipAssignment == IPAssignment::STATIC)
  {
    ETH.config(IPAddress(networkData->ip),
               IPAddress(networkData->gateway),
               IPAddress(networkData->subnet),
               IPAddress(networkData->dns));
  }
  Serial.println("WATCHDOG: ETH re-initialised, waiting for link...");
}

static void connectivityWatchdogTask(void *pvParameters)
{
  Serial.println("WATCHDOG: Started");
  bool ethResetDone = false;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(WATCHDOG_CHECK_INTERVAL_MS));

    if (ethernetConnected && mqttConnected && ntpSynced)
    {
      // All good — reset the timer and clear the reset flag
      lastReconnectAttempt = millis();
      ethResetDone = false;
      continue;
    }

    unsigned long downFor = millis() - lastReconnectAttempt;
    Serial.printf("WATCHDOG: Unhealthy for %lu ms (eth=%d mqtt=%d ntp=%d)\n",
                  downFor, (int)ethernetConnected, (int)mqttConnected, (int)ntpSynced);

    if (downFor >= WATCHDOG_REBOOT_TIMEOUT_MS)
    {
      Serial.println("WATCHDOG: 5 min timeout reached, rebooting...");
      delay(500);
      esp_restart();
    }
    else if (!ethResetDone && downFor >= WATCHDOG_ETH_RESET_MS)
    {
      Serial.println("WATCHDOG: 1 min timeout reached, resetting ETH...");
      resetEthernet();
      ethResetDone = true; // only reset once per unhealthy window
    }
  }
}

void setupWatchdog()
{
  lastReconnectAttempt = millis();

  xTaskCreatePinnedToCoreWithCaps(
      connectivityWatchdogTask,
      "conn_watchdog",
      4096,
      nullptr,
      2,
      nullptr,
      0,
      MALLOC_CAP_SPIRAM);
}