#pragma once

#include <Arduino.h>
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "esp_system.h"
#include "Properties.h"
#include "Validation.h"

bool keyHexToBytes(const char *hex, uint8_t key[16])
{
  if (!isValidEspNowKey(hex))
    return false;

  for (int i = 0; i < 16; i++)
  {
    char tmp[3];
    tmp[0] = hex[i * 2];
    tmp[1] = hex[i * 2 + 1];
    tmp[2] = 0;

    key[i] = strtoul(tmp, NULL, 16);
  }

  return true;
}

bool keyBytesToHex(const uint8_t key[16], char hex[33])
{
  if (!key || !hex)
    return false;

  for (int i = 0; i < 16; i++)
  {
    sprintf(hex + (i * 2), "%02X", key[i]);
  }

  hex[32] = '\0';

  return true;
}

bool macStringToBytes(const char *macStr, uint8_t mac[6])
{
  if (!isValidMacAddress(macStr))
    return false;

  for (int i = 0; i < 6; i++)
  {
    mac[i] = strtoul(macStr + (i * 3), NULL, 16);
  }

  return true;
}

bool macBytesToString(const uint8_t mac[6], char macStr[18])
{
  if (!mac || !macStr)
    return false;

  sprintf(macStr,
          "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);

  return true;
}

void delayedRestart()
{
  xTaskCreatePinnedToCoreWithCaps(
      [](void *param)
      {
        vTaskDelay(pdMS_TO_TICKS(1000));
        Serial.println("Rebooting now...");
        esp_restart();
        vTaskDelete(NULL);
      },
      "reboot_task",
      2048,
      NULL,
      1,
      NULL,
      1,
      MALLOC_CAP_INTERNAL);
}