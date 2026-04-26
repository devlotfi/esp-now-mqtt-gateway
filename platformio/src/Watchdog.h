#pragma once

#include "Vars.h"

#define WATCHDOG_CHECK_INTERVAL_MS (30UL * 1000UL)       // check every 30s
#define WATCHDOG_REBOOT_TIMEOUT_MS (5UL * 60UL * 1000UL) // reboot after 5min

static void connectivityWatchdogTask(void * /*pvParameters*/)
{
  Serial.println("WATCHDOG: Started");

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(WATCHDOG_CHECK_INTERVAL_MS));

    if (ethernetConnected && mqttConnected && ntpSynced)
    {
      // All good — reset the timer
      lastReconnectAttempt = millis();
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
  }
}

void setupWatchdog()
{
  lastReconnectAttempt = millis();

  xTaskCreatePinnedToCoreWithCaps(
      connectivityWatchdogTask,
      "conn_watchdog",
      2048,
      nullptr,
      2,
      nullptr,
      0,
      MALLOC_CAP_SPIRAM);
}