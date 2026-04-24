#include "settings_store.h"

bool SettingsStore::begin(const char *namespaceName)
{
  ready_ = preferences_.begin(namespaceName, false);
  if (!ready_)
  {
    Serial.println("[settings] Failed to open preferences namespace.");
  }
  return ready_;
}

AppSettings SettingsStore::load()
{
  AppSettings settings;
  if (!ready_)
  {
    Serial.println("[settings] Not ready, using defaults.");
    return settings;
  }

  uint32_t version = preferences_.getUInt("schema_v", settings.schemaVersion);
  if (version != settings.schemaVersion)
  {
    Serial.printf("[settings] Schema mismatch (stored=%lu expected=%lu), using defaults.\n", version, settings.schemaVersion);
    return settings;
  }

  settings.driveTuning.joystickDeadzonePercent = preferences_.getFloat("joy_deadzone", settings.driveTuning.joystickDeadzonePercent);
  settings.driveTuning.joystickExpo = preferences_.getFloat("joy_expo", settings.driveTuning.joystickExpo);
  settings.driveTuning.minStartLeftPercent = preferences_.getFloat("min_start_l", settings.driveTuning.minStartLeftPercent);
  settings.driveTuning.minStartRightPercent = preferences_.getFloat("min_start_r", settings.driveTuning.minStartRightPercent);
  settings.feedbackLedEnabled = preferences_.getBool("fx_led_on", settings.feedbackLedEnabled);
  settings.feedbackBuzzerEnabled = preferences_.getBool("fx_buzz_on", settings.feedbackBuzzerEnabled);

  sanitize(settings);
  Serial.println("[settings] Loaded settings from flash.");
  return settings;
}

bool SettingsStore::save(const AppSettings &settings)
{
  if (!ready_)
  {
    return false;
  }

  AppSettings safe = settings;
  sanitize(safe);

  bool ok = true;
  ok &= preferences_.putUInt("schema_v", safe.schemaVersion) > 0;
  ok &= preferences_.putFloat("joy_deadzone", safe.driveTuning.joystickDeadzonePercent) > 0;
  ok &= preferences_.putFloat("joy_expo", safe.driveTuning.joystickExpo) > 0;
  ok &= preferences_.putFloat("min_start_l", safe.driveTuning.minStartLeftPercent) > 0;
  ok &= preferences_.putFloat("min_start_r", safe.driveTuning.minStartRightPercent) > 0;
  ok &= preferences_.putBool("fx_led_on", safe.feedbackLedEnabled);
  ok &= preferences_.putBool("fx_buzz_on", safe.feedbackBuzzerEnabled);

  Serial.println(ok ? "[settings] Saved settings." : "[settings] Failed to save one or more settings.");
  return ok;
}

AppSettings SettingsStore::resetToDefaults()
{
  AppSettings defaults;
  if (ready_)
  {
    preferences_.clear();
    save(defaults);
  }
  Serial.println("[settings] Reset to defaults.");
  return defaults;
}

void SettingsStore::sanitize(AppSettings &settings) const
{
  settings.driveTuning.joystickDeadzonePercent = constrain(settings.driveTuning.joystickDeadzonePercent, 0.0f, 30.0f);
  settings.driveTuning.joystickExpo = constrain(settings.driveTuning.joystickExpo, 1.0f, 3.0f);
  settings.driveTuning.minStartLeftPercent = constrain(settings.driveTuning.minStartLeftPercent, 0.0f, 70.0f);
  settings.driveTuning.minStartRightPercent = constrain(settings.driveTuning.minStartRightPercent, 0.0f, 70.0f);
}
