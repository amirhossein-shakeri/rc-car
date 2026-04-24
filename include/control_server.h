#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <functional>

class ControlServer
{
public:
  using DriveHandler = std::function<void(int leftPercent, int rightPercent, float speedMultiplier, float leftToRightRatio)>;
  using JoystickHandler = std::function<void(int xPercent, int yPercent, float speedMultiplier, float leftToRightRatio)>;
  using StopHandler = std::function<void()>;

  explicit ControlServer(uint16_t port = 80);
  void begin(const DriveHandler &driveHandler, const JoystickHandler &joystickHandler, const StopHandler &stopHandler);
  void loop();

private:
  void registerRoutes();
  void handleIndex();
  void handleDrivePage();
  void handleDrive();
  void handleJoystick();
  void handleStop();
  int getIntArg(const char *name, int defaultValue);
  float getFloatArg(const char *name, float defaultValue);

  WebServer server_;
  DriveHandler driveHandler_;
  JoystickHandler joystickHandler_;
  StopHandler stopHandler_;
};
