#include <Arduino.h>

#include "control_server.h"
#include "feedback_controller.h"
#include "motor_controller.h"
#include "network_manager.h"
#include "settings_store.h"
#include "tb6612fng_motor_driver.h"

namespace
{
  constexpr TB6612FNGMotorPins kMotorDriverPins{
      23, // AIN1 -> left motor direction 1
      22, // AIN2 -> left motor direction 2
      21, // PWMA -> left motor PWM speed
      18, // BIN1 -> right motor direction 1
      19, // BIN2 -> right motor direction 2
      16, // PWMB -> right motor PWM speed
      17, // STBY -> driver standby/enable
  };

  constexpr TB6612FNGMotorChannels kMotorDriverChannels{
      0, // PWMA LEDC channel
      1, // PWMB LEDC channel
  };

  constexpr uint32_t kPwmFrequency = 20000;
  constexpr uint8_t kPwmResolution = 10;
  constexpr bool kEnableMotorBootSelfTest = true;
  constexpr char kApSsid[] = "RC-Car-ESP32";
  constexpr char kApPassword[] = "rc-car-2026";

  const FeedbackConfig kFeedbackConfig{
      false, // enableLed (optional; keep off by default for motor baseline debugging)
      false, // enableBuzzer (optional; keep off by default for motor baseline debugging)
      2,     // ledPin (NodeMCU-32S builtin LED commonly on GPIO2)
      true,  // ledActiveHigh
      4,     // ledPwmChannel
      15,    // buzzerPin
      false, // buzzerUsePwm (active buzzer modules commonly expect digital on/off)
      5,     // buzzerPwmChannel
      2000,  // buzzerFrequencyHz
      180,   // buzzerVolume (0-255)
      45,    // stopBeepMs
  };
} // namespace

TB6612FNGMotorDriver motorDriver(kMotorDriverPins, kMotorDriverChannels);
MotorController motorController(motorDriver);
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

void runMotorBootSelfTestIfEnabled()
{
  if (!kEnableMotorBootSelfTest)
  {
    return;
  }

  Serial.println("[boot] Motor self-test start (TB6612FNG forward/backward).");
  motorController.driveTankPercent(75, 75);
  delay(700);
  motorController.stop();
  delay(120);
  motorController.driveTankPercent(-75, -75);
  delay(700);
  motorController.stop();
  Serial.println("[boot] Motor self-test complete.");
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
  runMotorBootSelfTestIfEnabled();
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
