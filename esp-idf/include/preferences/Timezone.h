#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

struct TimezoneData
{
  bool isSet;
  char iana[TIMEZONE_IANA_SIZE];
  char timezonePosix[TIMEZONE_POSIX_SIZE];
};

static TimezoneData *timezoneDataCache = nullptr;

void saveTimezoneData(const TimezoneData *data)
{
  size_t written = preferences.putBytes("timezone_data", data, sizeof(TimezoneData));
  if (written != sizeof(TimezoneData))
    return;
  if (!timezoneDataCache)
    timezoneDataCache = (TimezoneData *)heap_caps_malloc(sizeof(TimezoneData), MALLOC_CAP_SPIRAM);
  if (timezoneDataCache)
    memcpy(timezoneDataCache, data, sizeof(TimezoneData));
}

TimezoneData *loadTimezoneData()
{
  if (timezoneDataCache)
    return timezoneDataCache;

  timezoneDataCache = (TimezoneData *)heap_caps_malloc(sizeof(TimezoneData), MALLOC_CAP_SPIRAM);
  if (!timezoneDataCache)
    return nullptr;
  timezoneDataCache->isSet = false;
  strlcpy(timezoneDataCache->timezonePosix, "UTC0", TIMEZONE_POSIX_SIZE);
  preferences.getBytes("timezone_data", timezoneDataCache, sizeof(TimezoneData));
  return timezoneDataCache;
}
