#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct NotificationsData
{
  bool isSet;
  char apiUrl[NOTIFICATIONS_API_URL_SIZE];
  char apiSecret[NOTIFICATIONS_API_SECRET_SIZE];
};

static NotificationsData *notificationsDataCache = nullptr;

void saveNotificationsData(const NotificationsData *data)
{
  size_t written = preferences.putBytes("notif_data", data, sizeof(NotificationsData));
  if (written != sizeof(NotificationsData))
    return;
  if (!notificationsDataCache)
    notificationsDataCache = (NotificationsData *)heap_caps_malloc(sizeof(NotificationsData), MALLOC_CAP_SPIRAM);
  if (notificationsDataCache)
    memcpy(notificationsDataCache, data, sizeof(NotificationsData));
}

NotificationsData *loadNotificationsData()
{
  if (notificationsDataCache)
    return notificationsDataCache;

  notificationsDataCache = (NotificationsData *)heap_caps_malloc(sizeof(NotificationsData), MALLOC_CAP_SPIRAM);
  if (!notificationsDataCache)
    return nullptr;
  notificationsDataCache->isSet = false;
  preferences.getBytes("notif_data", notificationsDataCache, sizeof(NotificationsData));
  return notificationsDataCache;
}