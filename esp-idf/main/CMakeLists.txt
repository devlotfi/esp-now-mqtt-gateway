idf_component_register(
  SRCS "main.cpp"
  INCLUDE_DIRS "../include"
  REQUIRES
    mqtt
    adafruit-neopixel
    ethernet
    etl
    ntp-client
)
