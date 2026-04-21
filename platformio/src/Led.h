#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"

static const uint8_t brightness = 32;
static const char *errorColor = "#FF0000";
static const char *successColor = "#00FF00";

void setLed(const char *hexColor, const uint8_t brightness)
{
  if (hexColor[0] == '#')
  {
    hexColor++;
  }

  // Convert hex string to 24-bit number
  uint32_t color = strtol(hexColor, NULL, 16);

  // Extract RGB components
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.setBrightness(brightness);
  pixel.show();
}

void setupLed()
{
  setLed(errorColor, brightness);
}

void updateLed()
{
  if (ethernetConnected && mqttConnected)
  {
    setLed(successColor, brightness);
  }
  else
  {
    setLed(errorColor, brightness);
  }
}