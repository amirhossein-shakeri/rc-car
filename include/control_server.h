#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <functional>

class ControlServer
{
public:
  using DriveHandler = std::function<void(int leftPercent, int rightPercent, float speedMultiplier, float leftToRightRatio)>;
  using StopHandler = std::function<void()>;

  explicit ControlServer(uint16_t port = 80);
  void begin(const DriveHandler &driveHandler, const StopHandler &stopHandler);
  void loop();

private:
  void registerRoutes();
  void handleIndex();
  void handleDrive();
  void handleStop();
  int getIntArg(const char *name, int defaultValue);
  float getFloatArg(const char *name, float defaultValue);

  WebServer server_;
  DriveHandler driveHandler_;
  StopHandler stopHandler_;
};
