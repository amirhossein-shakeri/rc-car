#include <Arduino.h>

#include "control_server.h"
#include "feedback_controller.h"
#include "motor_controller.h"
#include "network_manager.h"

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
  constexpr float kJoystickDeadzonePercent = 6.0f;
  constexpr float kJoystickExpo = 1.8f;
  constexpr float kJoystickMinStartLeftPercent = 30.0f;
  constexpr float kJoystickMinStartRightPercent = 30.0f;
  constexpr char kApSsid[] = "RC-Car-ESP32";
  constexpr char kApPassword[] = "rc-car-2026";

  const FeedbackConfig kFeedbackConfig{
      true, // enableLed
      true, // enableBuzzer
      2,    // ledPin (NodeMCU-32S builtin LED commonly on GPIO2)
      true, // ledActiveHigh
      4,    // ledPwmChannel
      15,   // buzzerPin
      45,   // stopBeepMs
  };
} // namespace

MotorController motorController(kPins, kChannels);
NetworkManager networkManager;
ControlServer controlServer(80);
FeedbackController feedbackController(kFeedbackConfig);

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("[boot] Tank-drive RC car booting...");

  motorController.begin(kPwmFrequency, kPwmResolution);
  motorController.setJoystickTuning(
      kJoystickDeadzonePercent,
      kJoystickExpo,
      kJoystickMinStartLeftPercent,
      kJoystickMinStartRightPercent);
  feedbackController.begin();
  feedbackController.playStartupSound();

  if (!networkManager.beginAccessPoint(kApSsid, kApPassword))
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
        motorController.setJoystickTuning(deadzonePercent, expo, minStartLeftPercent, minStartRightPercent);
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
