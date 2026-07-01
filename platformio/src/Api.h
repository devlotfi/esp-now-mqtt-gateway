#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <uri/UriBraces.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"
#include "Lookup.h"
#include "EspNow.h"
#include "controllers/AuthController.h"
#include "controllers/NotificationsController.h"
#include "controllers/EspNowController.h"
#include "controllers/PeerController.h"
#include "controllers/SleepyPeerController.h"
#include "controllers/DeviceController.h"
#include "controllers/NetworkController.h"
#include "controllers/GrafanaController.h"
#include "controllers/MqttController.h"
#include "middlewares/JwtMiddleware.h"

static AsyncWebServer server(80);
static AsyncCorsMiddleware cors;

void setupServer()
{
  // CORS
  cors.setOrigin("*");
  cors.setMethods("GET, POST, PUT, PATCH, DELETE, OPTIONS");
  cors.setHeaders("Content-Type, Authorization");
  cors.setAllowCredentials(false);
  server.addMiddleware(&cors);

  server.on(AsyncURIMatcher::exact("/api/auth/login"), HTTP_POST, AuthController::login);
  server.on(AsyncURIMatcher::exact("/api/auth/set-password"), HTTP_POST, AuthController::setPassword).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/device/reboot"), HTTP_POST, DeviceController::reboot).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/device/status"), HTTP_GET, DeviceController::status).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/network"), HTTP_GET, NetworkController::getConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/network"), HTTP_POST, NetworkController::setConfig).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/grafana"), HTTP_GET, GrafanaController::getConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/grafana"), HTTP_POST, GrafanaController::setConfig).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/mqtt"), HTTP_GET, MqttController::getConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/mqtt"), HTTP_POST, MqttController::setConfig).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/notifications"), HTTP_GET, NotificationsController::getConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/notifications"), HTTP_POST, NotificationsController::setConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/notifications/test"), HTTP_POST, NotificationsController::test).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/esp-now"), HTTP_GET, EspNowController::getConfig).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/esp-now"), HTTP_POST, EspNowController::setConfig).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/peers"), HTTP_GET, PeerController::peers).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/peers"), HTTP_POST, PeerController::addPeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/api/peers/([0-9a-fA-F-]{36})$"), HTTP_DELETE, PeerController::deletePeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/api/peers/([0-9a-fA-F-]{36})/topics$"), HTTP_GET, PeerController::topics).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/api/peers/([0-9a-fA-F-]{36})/topics$"), HTTP_POST, PeerController::addTopic).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/api/peers/([0-9a-fA-F-]{36})/topics$"), HTTP_DELETE, PeerController::deleteTopic).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/api/sleepy-peers"), HTTP_GET, SleepyPeerController::sleepyPeers).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/api/sleepy-peers"), HTTP_POST, SleepyPeerController::addSleepyPeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/api/sleepy-peers/([0-9a-fA-F-]{36})$"), HTTP_DELETE, SleepyPeerController::deleteSleepyPeer).addMiddleware(&jwtAuth);

  server.begin();
}