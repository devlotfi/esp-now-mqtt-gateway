idf_component_register(
  SRCS 
    "Adafruit_NeoPixel.cpp"
    "Adafruit_Neopixel_RP2.cpp"
    "esp.c"
    "esp8266.c"
    "kendyte_k210.c"
    "psoc6.c"
  INCLUDE_DIRS "."
  REQUIRES espressif__arduino-esp32
)
