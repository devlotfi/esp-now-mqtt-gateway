#pragma once

#include "Vars.h"

void corsMiddleware(void (*next)())
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  if (server.method() == HTTP_OPTIONS)
  {
    server.send(204);
    return;
  }
  next();
}