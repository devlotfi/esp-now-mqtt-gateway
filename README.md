<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/github-banner.png">

# 📜 esp-now-mqtt-gateway

An app to controll IOT devices using MQTT

# 📌 Contents

- [Tech stack](#tech-stack)
- [How does the system work ?](#how-does-the-system-work-)
- [Build](#build)
- [Images](#images)
- [Web App](#web-app)

# Tech stack

## App

<p float="left">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/html.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/css.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/ts.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/tailwind.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/react.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/lucide.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/formik.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/i18n.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/tanstack-router.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/tanstack-query.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/heroui.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/vite.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/openapi.svg">
</p>

## Services

<p float="left">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/gemini.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/netlify.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/push-notifier.svg">
</p>

## IOT

<p float="left">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/arduino.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/espressif.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/mqtt.svg">
</p>

## Diagrams

<p float="left">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/drawio.svg">
  <img height="50px" src="https://devlotfi.github.io/stack-icons/icons/fritzing.svg">
</p>

# How does the system work ?

- The gateway connects to the MQTT Broker and subscribes to the defined topics and forwards recived messages from ESP-NOW to MQTT
- Devices are categorized into 2 types **Normal Devices** and **Sleepy Devices**
- Normal devices send and recive messages directly via the gateway similar to a direct MQTT connection
- Sleepy Devices periodicly send their data that will be saved as a Retained message on the data topic
- Sleepy Devices recive commands by sending the command the command topic, which then gets stored in the gateways until the devices requests it
- To communicate with the gateway you can use the dedicated library in the [Releases](https://github.com/devlotfi/esp-now-mqtt-gateway/releases)
- Devices can also send notifications via the gateway using [PushNotifier](https://github.com/devlotfi/push-notifier)
- For a UI Dashboard you can use [EspCommander](https://github.com/devlotfi/esp-commander)
- To configure the gateway use the Admin Dashboard Web App

<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/working-diagram.png">

# Build

## Components

- ESP32S3 (N16R8)
- W5500 Ethernet Module
- Cables
- Case (Optional)

## Connections

| W5500         | ESP32S3 (N16R8) |
|---------------|-----------------|
| 3.3v          | 3.3v            |
| GND           | GND             |
| CS            | 10              |
| RST           | 9               |
| MISO          | 13              |
| MOSI          | 11              |
| SCLK          | 12              |
| INT           | 15              |

<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/fritzing.png">

## Images

Some images of the final build

<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/main/github-assets/build-1.jpg">

# Web App

<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-1.png">
<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-2.png">
<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-3.png">
<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-4.png">
<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-5.png">
<img src="https://raw.githubusercontent.com/devlotfi/esp-now-mqtt-gateway/master/github-assets/preview-6.png">
