#pragma once

#include <stdint.h>
#include "Properties.h"
#include "Vars.h"
#include "Utils.h"

class IPAssignment
{
public:
  enum IPAssignmentEnum
  {
    DHCP,
    STATIC,
    UNKNOWN // The third case for no match
  };

  IPAssignmentEnum val;
  constexpr IPAssignment(IPAssignmentEnum v) : val(v) {}
  constexpr operator IPAssignmentEnum() const { return val; }
  explicit operator bool() = delete;

  const char *to_string() const
  {
    switch (val)
    {
    case DHCP:
      return "DHCP";
    case STATIC:
      return "STATIC";
    case UNKNOWN:
      return "UNKNOWN";
    default:
      return "UNKNOWN";
    }
  }

  static IPAssignment from_string(const char *str)
  {
    if (str != nullptr)
    {
      if (strcmp(str, "DHCP") == 0)
        return DHCP;
      if (strcmp(str, "STATIC") == 0)
        return STATIC;
    }
    return UNKNOWN; // No match found
  }
};

struct NetworkData
{
  IPAssignment ipAssignment;
  uint32_t ip;
  uint32_t gateway;
  uint32_t subnet;
  uint32_t dns;
};

void saveNetworkData(const NetworkData *data)
{
  preferences.putBytes("network_data", data, sizeof(NetworkData));
}

NetworkData *loadNetworkData()
{
  NetworkData *data = (NetworkData *)malloc(sizeof(NetworkData));
  if (!data)
    return nullptr;
  data->ipAssignment = IPAssignment::DHCP;
  preferences.getBytes("network_data", data, sizeof(NetworkData));
  return data;
}
