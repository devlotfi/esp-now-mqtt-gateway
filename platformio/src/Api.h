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
#include "NvsData.h"
#include "Lookup.h"
#include "EspNow.h"
#include "controllers/AuthController.h"
#include "controllers/NotificationsController.h"
#include "controllers/PeerController.h"
#include "middlewares/JwtMiddleware.h"

static AsyncCorsMiddleware cors;

void setupServer()
{
  // CORS
  cors.setOrigin("*");
  cors.setMethods("GET, POST, PUT, PATCH, DELETE, OPTIONS");
  cors.setHeaders("Content-Type, Authorization");
  cors.setAllowCredentials(false);
  server.addMiddleware(&cors);

  server.on(AsyncURIMatcher::exact("/login"), HTTP_POST, AuthController::login);
  server.on(AsyncURIMatcher::exact("/set-password"), HTTP_POST, AuthController::setPassword).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/set-pmk"), HTTP_POST, AuthController::setPMK).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/notifications"), HTTP_POST, NotificationsController::configure).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/notifications/test"), HTTP_POST, NotificationsController::test).addMiddleware(&jwtAuth);

  server.on(AsyncURIMatcher::exact("/peers"), HTTP_GET, PeerController::peers).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::exact("/peers"), HTTP_POST, PeerController::addPeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/peers/([0-9a-fA-F-]{36})$"), HTTP_DELETE, PeerController::deletePeer).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_GET, PeerController::topics).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_POST, PeerController::addTopic).addMiddleware(&jwtAuth);
  server.on(AsyncURIMatcher::regex("^/topics/([0-9a-fA-F-]{36})$"), HTTP_DELETE, PeerController::deleteTopic).addMiddleware(&jwtAuth);

  server.begin();
}