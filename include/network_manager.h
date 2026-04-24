#pragma once

#include <Arduino.h>
#include <WiFi.h>

class NetworkManager
{
public:
  bool beginAccessPoint(const char *ssid, const char *password, uint8_t channel = 1, bool hidden = false, uint8_t maxConnections = 4);

private:
  void logNetworkDetails() const;
};
