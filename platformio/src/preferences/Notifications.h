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

void saveNotificationsData(const NotificationsData *data)
{
  preferences.putBytes("notif_data", data, sizeof(NotificationsData));
}

NotificationsData *loadNotificationsData()
{
  NotificationsData *data = (NotificationsData *)malloc(sizeof(NotificationsData));
  if (!data)
    return nullptr;
  data->isSet = false;
  preferences.getBytes("notif_data", data, sizeof(NotificationsData));
  return data;
}