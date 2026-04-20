#pragma once

#include <Arduino.h>
#include <lwip/sockets.h>
#include "esp_system.h"
#include "Properties.h"

bool isValidIPv4(const char *ip)
{
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
  return result == 1;
}

bool isPrivateIPv4(const char *ip, bool strict = false)
{
  if (!ip)
    return false;

  struct sockaddr_in sa;
  if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 1)
    return false;

  uint32_t addr = ntohl(sa.sin_addr.s_addr);
  uint8_t first = (addr >> 24) & 0xFF;
  uint8_t second = (addr >> 16) & 0xFF;

  if (strict)
  {
    if (first == 127)
      return false; // loopback
    if (first == 169 && second == 254)
      return false; // link-local
    if (addr == 0)
      return false; // 0.0.0.0
  }

  if (first == 10)
    return true; // 10.0.0.0/8
  if (first == 172 && second >= 16 && second <= 31)
    return true; // 172.16.0.0/12
  if (first == 192 && second == 168)
    return true; // 192.168.0.0/16

  return false;
}

bool isValidSubnetMask(const char *mask)
{
  struct in_addr addr;

  // First: valid IPv4 format
  if (inet_pton(AF_INET, mask, &addr) != 1)
    return false;

  uint32_t m = ntohl(addr.s_addr); // convert to host order

  // Reject all 0s and all 1s (optional but recommended)
  if (m == 0x00000000 || m == 0xFFFFFFFF)
    return false;

  // Check for contiguous ones: valid masks look like 111...1100...00
  // Trick: invert and check it's power-of-two minus 1
  uint32_t inv = ~m;

  // inv must be like 000...0011...11
  if ((inv & (inv + 1)) != 0)
    return false;

  return true;
}

static bool isValidUrl(const char *url)
{
  if (!url)
    return false;

  size_t len = strlen(url);
  if (len < 10 || len > 200)
    return false;

  // Must start with http:// or https://
  if (strncmp(url, "http://", 7) != 0 &&
      strncmp(url, "https://", 8) != 0)
    return false;

  const char *p = strstr(url, "://");
  if (!p)
    return false;

  p += 3; // skip ://

  // Host must exist
  if (*p == '\0')
    return false;

  while (*p && *p != '/' && *p != ':')
  {
    char c = *p;

    if (!(
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-'))
      return false;

    p++;
  }

  // Optional port
  if (*p == ':')
  {
    p++;
    if (!isdigit(*p))
      return false;

    while (*p && *p != '/')
    {
      if (!isdigit(*p))
        return false;
      p++;
    }
  }

  return true;
}

static bool isValidMqttUrl(const char *url)
{
  if (!url)
    return false;

  size_t len = strlen(url);
  if (len < 12 || len > 200)
    return false;

  // Must start with mqtt:// or mqtts://
  bool isSecure = false;

  if (strncmp(url, "mqtt://", 7) == 0)
  {
    isSecure = false;
  }
  else if (strncmp(url, "mqtts://", 8) == 0)
  {
    isSecure = true;
  }
  else
  {
    return false;
  }

  const char *p = strstr(url, "://");
  if (!p)
    return false;

  p += 3; // skip ://

  // Host must exist
  if (*p == '\0')
    return false;

  // Validate hostname
  while (*p && *p != '/' && *p != ':')
  {
    char c = *p;

    if (!(
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-'))
      return false;

    p++;
  }

  // Optional port (recommended for MQTT)
  if (*p == ':')
  {
    p++;

    if (!isdigit(*p))
      return false;

    int port = 0;

    while (*p && *p != '/')
    {
      if (!isdigit(*p))
        return false;

      port = port * 10 + (*p - '0');
      p++;
    }

    // Validate port range
    if (port <= 0 || port > 65535)
      return false;
  }
  else
  {
    return false;
  }

  return true;
}

static bool isHexChar(char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool isValidEspNowKey(const char *key)
{
  if (!key)
    return false;

  size_t len = strlen(key);

  if (len != 32)
    return false;

  for (size_t i = 0; i < 32; i++)
  {
    if (!isHexChar(key[i]))
      return false;
  }

  return true;
}

bool isValidMacAddress(const char *mac)
{
  if (!mac)
    return false;

  if (strlen(mac) != 17)
    return false;

  for (int i = 0; i < 17; i++)
  {
    if ((i + 1) % 3 == 0)
    {
      if (mac[i] != ':')
        return false;
    }
    else
    {
      if (!isHexChar(mac[i]))
        return false;
    }
  }

  return true;
}

bool isUnicastMacAddress(const char *mac)
{
  if (!isValidMacAddress(mac))
    return false;

  // Reject broadcast
  if (strcasecmp(mac, "FF:FF:FF:FF:FF:FF") == 0)
    return false;

  // Reject all zeros
  if (strcasecmp(mac, "00:00:00:00:00:00") == 0)
    return false;

  // First byte
  char byteStr[3] = {mac[0], mac[1], '\0'};
  uint8_t firstByte = strtoul(byteStr, nullptr, 16);

  // Reject multicast
  if (firstByte & 0x01)
    return false;

  return true;
}

bool isValidWifiChannel(uint8_t channel)
{
  if (channel >= 1 && channel <= 13)
  {
    return true;
  }
  return false;
}