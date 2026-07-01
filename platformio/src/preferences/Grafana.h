#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct GrafanaData
{
  bool isSet;
  char url[GRAFANA_URL_SIZE];
  char instanceId[GRAFANA_INSTANCE_ID_SIZE];
  char apiKey[GRAFANA_API_KEY_SIZE];
};

static GrafanaData *grafanaDataCache = nullptr;

void saveGrafanaData(const GrafanaData *data)
{
  size_t written = preferences.putBytes("grafana_data", data, sizeof(GrafanaData));
  if (written != sizeof(GrafanaData))
    return;
  if (!grafanaDataCache)
    grafanaDataCache = (GrafanaData *)malloc(sizeof(GrafanaData));
  if (grafanaDataCache)
    memcpy(grafanaDataCache, data, sizeof(GrafanaData));
}

GrafanaData *loadGrafanaData()
{
  if (grafanaDataCache)
    return grafanaDataCache;

  grafanaDataCache = (GrafanaData *)malloc(sizeof(GrafanaData));
  if (!grafanaDataCache)
    return nullptr;
  grafanaDataCache->isSet = false;
  preferences.getBytes("grafana_data", grafanaDataCache, sizeof(GrafanaData));
  return grafanaDataCache;
}
