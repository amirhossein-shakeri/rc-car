#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <functional>

class ControlServer
{
public:
  using DriveHandler = std::function<void(int leftPercent, int rightPercent, float speedMultiplier, float leftToRightRatio)>;
  using JoystickHandler = std::function<void(int xPercent, int yPercent, float speedMultiplier, float leftToRightRatio)>;
  using TuneHandler = std::function<void(float deadzonePercent, float expo, float minStartLeftPercent, float minStartRightPercent)>;
  using StopHandler = std::function<void()>;

  explicit ControlServer(uint16_t port = 80);
  void begin(const DriveHandler &driveHandler, const JoystickHandler &joystickHandler, const TuneHandler &tuneHandler, const StopHandler &stopHandler);
  void loop();

private:
  void registerRoutes();
  void handleIndex();
  void handleDrivePage();
  void handleTunePage();
  void handleDrive();
  void handleJoystick();
  void handleTune();
  void handleStop();
  int getIntArg(const char *name, int defaultValue);
  float getFloatArg(const char *name, float defaultValue);

  WebServer server_;
  DriveHandler driveHandler_;
  JoystickHandler joystickHandler_;
  TuneHandler tuneHandler_;
  StopHandler stopHandler_;
};
