#include <Arduino.h>

#include "control_server.h"
#include "feedback_controller.h"
#include "motor_controller.h"
#include "network_manager.h"
#include "settings_store.h"

namespace
{
  constexpr MotorPins kPins{
      23,
      22,
      18,
      19,
  };

  constexpr MotorChannels kChannels{
      0,
      1,
      2,
      3,
  };

  constexpr uint32_t kPwmFrequency = 20000;
  constexpr uint8_t kPwmResolution = 10;
  constexpr char kApSsid[] = "RC-Car-ESP32";
  constexpr char kApPassword[] = "rc-car-2026";

  const FeedbackConfig kFeedbackConfig{
      true,  // enableLed
      true,  // enableBuzzer
      2,     // ledPin (NodeMCU-32S builtin LED commonly on GPIO2)
      true,  // ledActiveHigh
      4,     // ledPwmChannel
      15,    // buzzerPin
      true,  // buzzerUsePwm
      5,     // buzzerPwmChannel
      2000,  // buzzerFrequencyHz
      180,   // buzzerVolume (0-255)
      45,    // stopBeepMs
  };
} // namespace

MotorController motorController(kPins, kChannels);
NetworkManager networkManager;
ControlServer controlServer(80);
FeedbackController feedbackController(kFeedbackConfig);
SettingsStore settingsStore;
AppSettings appSettings;

void applyDriveTuningFromSettings()
{
  motorController.setJoystickTuning(
      appSettings.driveTuning.joystickDeadzonePercent,
      appSettings.driveTuning.joystickExpo,
      appSettings.driveTuning.minStartLeftPercent,
      appSettings.driveTuning.minStartRightPercent);
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("[boot] Tank-drive RC car booting...");

  settingsStore.begin();
  appSettings = settingsStore.load();

  motorController.begin(kPwmFrequency, kPwmResolution);
  applyDriveTuningFromSettings();
  feedbackController.begin();
  feedbackController.playStartupSound();

  if (networkManager.beginAccessPoint(kApSsid, kApPassword))
  {
    feedbackController.playWifiConnectedSound();
  }
  else
  {
    Serial.println("[boot] Hotspot failed to start. Retrying in loop...");
  }

  controlServer.begin(
      [](int leftPercent, int rightPercent, float speedMultiplier, float leftToRightRatio)
      {
        motorController.setSpeedMultiplier(speedMultiplier);
        motorController.setLeftToRightRatio(leftToRightRatio);
        motorController.driveTankPercent(leftPercent, rightPercent);
        feedbackController.updateFromMotor(motorController.getDutyState());
      },
      [](int xPercent, int yPercent, float speedMultiplier, float leftToRightRatio)
      {
        motorController.setSpeedMultiplier(speedMultiplier);
        motorController.setLeftToRightRatio(leftToRightRatio);
        motorController.driveJoystickPercent(xPercent, yPercent);
        feedbackController.updateFromMotor(motorController.getDutyState());
      },
      [](float deadzonePercent, float expo, float minStartLeftPercent, float minStartRightPercent)
      {
        appSettings.driveTuning.joystickDeadzonePercent = deadzonePercent;
        appSettings.driveTuning.joystickExpo = expo;
        appSettings.driveTuning.minStartLeftPercent = minStartLeftPercent;
        appSettings.driveTuning.minStartRightPercent = minStartRightPercent;
        applyDriveTuningFromSettings();
      },
      [](float &deadzonePercent, float &expo, float &minStartLeftPercent, float &minStartRightPercent)
      {
        deadzonePercent = appSettings.driveTuning.joystickDeadzonePercent;
        expo = appSettings.driveTuning.joystickExpo;
        minStartLeftPercent = appSettings.driveTuning.minStartLeftPercent;
        minStartRightPercent = appSettings.driveTuning.minStartRightPercent;
      },
      []()
      {
        settingsStore.save(appSettings);
        feedbackController.playTuneSavedSound();
      },
      []()
      {
        appSettings = settingsStore.resetToDefaults();
        applyDriveTuningFromSettings();
        feedbackController.playTuneSavedSound();
      },
      []()
      {
        motorController.stop();
        feedbackController.updateFromMotor(motorController.getDutyState());
      });

  Serial.println("[boot] Setup complete.");
}

void loop()
{
  controlServer.loop();
  feedbackController.loop();
}
