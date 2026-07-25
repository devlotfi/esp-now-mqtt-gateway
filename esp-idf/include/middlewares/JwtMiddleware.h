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

AsyncMiddlewareFunction jwtAuth([](AsyncWebServerRequest *request, ArMiddlewareNext next)
                                {
  if (!request->hasHeader("Authorization")) {
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
    return;
  }
  String header = request->getHeader("Authorization")->value();
  if (!header.startsWith("Bearer ")) {
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
    return;
  }
  String token = header.substring(7);
  if (!verifyToken(token.c_str())) {
    request->send(401, "application/json", "{\"error\":\"UNAUTHORIZED\"}");
    return;
  }
  request->setAttribute("jwt", token);
  next(); });