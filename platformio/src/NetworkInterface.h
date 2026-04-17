#pragma once

#include "preferences/Network.h"
#include "Mqtt.h"

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

void setupNetwork()
{
  NetworkData *networkData = loadNetworkData();

  Network.onEvent(WiFiEvent);

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

  ETH.begin(ETH_PHY_W5500, 1, WIZNET_CS_PIN, WIZNET_INT_PIN, WIZNET_RESET_PIN, SPI2_HOST,
            WIZNET_SCLK_PIN, WIZNET_MISO_PIN, WIZNET_MOSI_PIN);

  free(networkData);
}