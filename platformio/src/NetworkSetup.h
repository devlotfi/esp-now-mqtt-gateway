#pragma once

#include <AsyncTCP.h>
#include <WiFi.h>
#include <ETH.h>
#include <NTPClient.h>
#include "preferences/Network.h"
#include "preferences/Timezone.h"
#include "Mqtt.h"
#include "Led.h"
#include "Vars.h"

#define NTP_UPDATE_TASK_STACK_SIZE 4096
#define NTP_UPDATE_INTERVAL_MS 60000

static NTPClient timeClient(networkUDP, "pool.ntp.org", 0, 60000);

static void ntpUpdateTask(void *pvParameters)
{
  for (;;)
  {
    if (ethernetConnected)
    {
      timeClient.update();
    }

    vTaskDelay(pdMS_TO_TICKS(NTP_UPDATE_INTERVAL_MS));
  }
}

// Call once from setup().
void initNtpUpdateTask()
{
  xTaskCreatePinnedToCoreWithCaps(
      ntpUpdateTask,
      "ntp_update_worker",
      NTP_UPDATE_TASK_STACK_SIZE,
      nullptr,
      1,
      nullptr,
      tskNO_AFFINITY,
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

void syncTime()
{
  Serial.println("NTP: Syncing time...");

  TimezoneData *timezoneData = loadTimezoneData();

  // Keep the system clock in UTC
  timeClient.setTimeOffset(0);
  timeClient.begin();

  while (!timeClient.forceUpdate())
  {
    Serial.println("NTP: Waiting for sync...");
    delay(2000);
  }

  time_t epoch = timeClient.getEpochTime();

  struct timeval tv = {
      .tv_sec = epoch,
      .tv_usec = 0,
  };

  settimeofday(&tv, nullptr);

  // Configure local timezone
  setenv("TZ", timezoneData->timezonePosix, 1);
  tzset();
  Serial.printf("POSIX TZ: %s\n", timezoneData->timezonePosix);

  ntpSynced = true;

  printCurrentTime();
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
    Serial.print("ETH: IP -> ");
    Serial.println(ETH.localIP());
    ethernetConnected = true;
    updateLed();
    syncTime();
    if (!mqttStarted)
    {
      Serial.println("MQTT: Starting client...");
      startMqtt();
      mqttStarted = true;
      updateLed();
    }
    break;

  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH: Link down");
    ethernetConnected = false;
    updateLed();
    break;

  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH: Stopped");
    ethernetConnected = false;
    updateLed();
    break;

  default:
    break;
  }
}

void setupNetwork()
{
  NetworkData *networkData = loadNetworkData();

  Network.onEvent(WiFiEvent);

  ETH.begin(ETH_PHY_W5500, 1, WIZNET_CS_PIN, WIZNET_INT_PIN, WIZNET_RESET_PIN, SPI2_HOST,
            WIZNET_SCLK_PIN, WIZNET_MISO_PIN, WIZNET_MOSI_PIN);

  if (networkData->ipAssignment == IPAssignment::STATIC)
  {
    IPAddress ip(networkData->ip);
    IPAddress gateway(networkData->gateway);
    IPAddress subnet(networkData->subnet);
    IPAddress dns(networkData->dns);

    Serial.println("NETWORK-INTERFACE: Using static config");
    Serial.printf("ip: %s\n", ip.toString());
    Serial.printf("gateway: %s\n", gateway.toString());
    Serial.printf("subnet: %s\n", subnet.toString());
    Serial.printf("dns: %s\n", dns.toString());

    ETH.config(ip, gateway, subnet, dns);
  }
  else
  {
    Serial.println("NETWORK-INTERFACE: Using DHCP");
  }
}