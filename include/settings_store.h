#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct DriveTuningSettings
{
  float joystickDeadzonePercent = 6.0f;
  float joystickExpo = 1.8f;
  float minStartLeftPercent = 30.0f;
  float minStartRightPercent = 30.0f;
};

struct AppSettings
{
  DriveTuningSettings driveTuning;
  bool feedbackLedEnabled = true;
  bool feedbackBuzzerEnabled = true;
  uint32_t schemaVersion = 1;
};

class SettingsStore
{
public:
  bool begin(const char *namespaceName = "rc-car");
  AppSettings load();
  bool save(const AppSettings &settings);
  AppSettings resetToDefaults();

private:
  void sanitize(AppSettings &settings) const;

  Preferences preferences_;
  bool ready_ = false;
};
