#pragma once

#include <Arduino.h>

#include "motor_controller.h"

struct FeedbackConfig
{
  bool enableLed = true;
  bool enableBuzzer = true;
  uint8_t ledPin = 2;
  bool ledActiveHigh = true;
  uint8_t ledPwmChannel = 4;
  uint8_t buzzerPin = 15;
  uint8_t stopBeepMs = 45;

  FeedbackConfig() = default;
  FeedbackConfig(
      bool enableLedValue,
      bool enableBuzzerValue,
      uint8_t ledPinValue,
      bool ledActiveHighValue,
      uint8_t ledPwmChannelValue,
      uint8_t buzzerPinValue,
      uint8_t stopBeepMsValue)
      : enableLed(enableLedValue),
        enableBuzzer(enableBuzzerValue),
        ledPin(ledPinValue),
        ledActiveHigh(ledActiveHighValue),
        ledPwmChannel(ledPwmChannelValue),
        buzzerPin(buzzerPinValue),
        stopBeepMs(stopBeepMsValue)
  {
  }
};

class FeedbackController
{
public:
  explicit FeedbackController(const FeedbackConfig &config);

  void begin();
  void playStartupSound();
  void updateFromMotor(const MotorDutyState &dutyState);
  void loop();

private:
  void applyLed(int brightness);
  void triggerStopBeep();

  FeedbackConfig config_;
  bool wasMoving_ = false;
  bool buzzerActive_ = false;
  unsigned long buzzerOffAtMs_ = 0;
  int ledBrightness_ = 0;
  bool stopBlinkState_ = false;
  unsigned long lastBlinkAtMs_ = 0;
};
