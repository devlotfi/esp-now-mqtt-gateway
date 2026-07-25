#pragma once

#include <ETH.h>
#include "Properties.h"
#include "Vars.h"

IPAddress getBroadcastAddress()
{
  IPAddress ip = ETH.localIP();
  IPAddress subnet = ETH.subnetMask();

  return IPAddress(
      ip[0] | (~subnet[0] & 0xFF),
      ip[1] | (~subnet[1] & 0xFF),
      ip[2] | (~subnet[2] & 0xFF),
      ip[3] | (~subnet[3] & 0xFF));
}

bool sendWakeOnLan(const uint16_t port, const uint8_t targetMac[6])
{
  uint8_t packet[102];

  memset(packet, 0xFF, 6);

  for (int i = 0; i < 16; i++)
  {
    memcpy(&packet[6 + i * 6], targetMac, 6);
  }

  IPAddress broadcast = getBroadcastAddress();

  Serial.printf("WOL: Broadcast %s\n",
                broadcast.toString().c_str());

  if (!networkUDP.begin(0))
  {
    Serial.println("WOL: Failed to open UDP socket");
    return false;
  }

  bool success = networkUDP.beginPacket(broadcast, port);

  if (!success)
  {
    Serial.println("WOL: beginPacket failed");
    networkUDP.stop();
    return false;
  }

  networkUDP.write(packet, sizeof(packet));
  networkUDP.endPacket();
  networkUDP.stop();

  Serial.println("WOL: Magic packet sent");

  return true;
}