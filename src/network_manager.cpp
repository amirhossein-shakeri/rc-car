#include "network_manager.h"

bool NetworkManager::beginAccessPoint(const char *ssid, const char *password, uint8_t channel, bool hidden, uint8_t maxConnections)
{
  Serial.printf("[net] Starting ESP32 hotspot SSID='%s' channel=%u hidden=%d maxClients=%u\n", ssid, channel, hidden, maxConnections);
  bool started = WiFi.softAP(ssid, password, channel, hidden, maxConnections);
  if (!started)
  {
    Serial.println("[net] Failed to start hotspot.");
    return false;
  }

  logNetworkDetails();
  return true;
}

void NetworkManager::logNetworkDetails() const
{
  IPAddress apIp = WiFi.softAPIP();
  Serial.printf("[net] Hotspot ready. Connect phone to Wi-Fi and open http://%s\n", apIp.toString().c_str());
  Serial.printf("[net] AP IP=%s, connectedStations=%d\n", apIp.toString().c_str(), WiFi.softAPgetStationNum());
}
