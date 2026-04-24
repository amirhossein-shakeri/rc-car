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
  bool buzzerUsePwm = true;
  uint8_t buzzerPwmChannel = 5;
  uint16_t buzzerFrequencyHz = 2000;
  uint8_t buzzerVolume = 160;
  uint8_t stopBeepMs = 45;

  FeedbackConfig() = default;
  FeedbackConfig(
      bool enableLedValue,
      bool enableBuzzerValue,
      uint8_t ledPinValue,
      bool ledActiveHighValue,
      uint8_t ledPwmChannelValue,
      uint8_t buzzerPinValue,
      bool buzzerUsePwmValue,
      uint8_t buzzerPwmChannelValue,
      uint16_t buzzerFrequencyHzValue,
      uint8_t buzzerVolumeValue,
      uint8_t stopBeepMsValue)
      : enableLed(enableLedValue),
        enableBuzzer(enableBuzzerValue),
        ledPin(ledPinValue),
        ledActiveHigh(ledActiveHighValue),
        ledPwmChannel(ledPwmChannelValue),
        buzzerPin(buzzerPinValue),
        buzzerUsePwm(buzzerUsePwmValue),
        buzzerPwmChannel(buzzerPwmChannelValue),
        buzzerFrequencyHz(buzzerFrequencyHzValue),
        buzzerVolume(buzzerVolumeValue),
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
  void playWifiConnectedSound();
  void playTuneSavedSound();
  void updateFromMotor(const MotorDutyState &dutyState);
  void loop();

private:
  struct BeepNote
  {
    uint16_t frequencyHz = 0;
    uint8_t volume = 0;
    uint16_t durationMs = 0;
    uint16_t pauseAfterMs = 0;
  };

  void queueNote(uint16_t frequencyHz, uint8_t volume, uint16_t durationMs, uint16_t pauseAfterMs);
  void startQueuedNote(const BeepNote &note);
  void stopBuzzer();
  void runBuzzerQueue();
  void applyLed(int brightness);
  void triggerStopBeep();

  FeedbackConfig config_;
  bool wasMoving_ = false;
  bool buzzerActive_ = false;
  unsigned long buzzerOffAtMs_ = 0;
  int ledBrightness_ = 0;
  bool stopBlinkState_ = false;
  unsigned long lastBlinkAtMs_ = 0;
  static const uint8_t kMaxQueuedNotes = 10;
  BeepNote noteQueue_[kMaxQueuedNotes];
  uint8_t noteHead_ = 0;
  uint8_t noteTail_ = 0;
  bool notePlaying_ = false;
  unsigned long noteEndsAtMs_ = 0;
  unsigned long notePauseEndsAtMs_ = 0;
};
