#pragma once

#include <Arduino.h>

struct MotorPins
{
  uint8_t leftA;
  uint8_t leftB;
  uint8_t rightA;
  uint8_t rightB;
};

struct MotorChannels
{
  uint8_t leftA;
  uint8_t leftB;
  uint8_t rightA;
  uint8_t rightB;
};

struct MotorDutyState
{
  int leftDuty = 0;
  int rightDuty = 0;
  int maxDuty = 1023;
};

class MotorController
{
public:
  MotorController(const MotorPins &pins, const MotorChannels &channels);

  void begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits);
  void setSpeedMultiplier(float multiplier);
  void setLeftToRightRatio(float ratio);
  void setJoystickTuning(float deadzonePercent, float expo, float minStartLeftPercent, float minStartRightPercent);
  void driveTankPercent(int leftPercent, int rightPercent);
  void driveJoystickPercent(int xPercent, int yPercent);
  void stop();
  MotorDutyState getDutyState() const;

private:
  float applyDeadzoneExpo(float inputPercent) const;
  int applyMinStartPercent(int speedPercent, float minStartPercent) const;
  void runMotor(uint8_t channelA, uint8_t channelB, int speed);

  MotorPins pins_;
  MotorChannels channels_;
  uint32_t pwmFrequency_ = 20000;
  uint8_t pwmResolutionBits_ = 10;
  int maxDuty_ = 1023;
  float speedMultiplier_ = 1.0f;
  float leftToRightRatio_ = 1.0f;
  float joystickDeadzonePercent_ = 6.0f;
  float joystickExpo_ = 1.7f;
  float minStartLeftPercent_ = 30.0f;
  float minStartRightPercent_ = 30.0f;
  int lastLeftDuty_ = 0;
  int lastRightDuty_ = 0;
};
