#pragma once

#include <Arduino.h>

#include "motor_driver.h"

struct L9110SMotorPins
{
  uint8_t leftA;
  uint8_t leftB;
  uint8_t rightA;
  uint8_t rightB;
};

struct L9110SMotorChannels
{
  uint8_t leftA;
  uint8_t leftB;
  uint8_t rightA;
  uint8_t rightB;
};

class L9110SMotorDriver : public MotorDriver
{
public:
  L9110SMotorDriver(const L9110SMotorPins &pins, const L9110SMotorChannels &channels);

  void begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits) override;
  void driveDuties(int leftDuty, int rightDuty) override;
  void stop() override;
  int maxDuty() const override;
  const char *name() const override;

private:
  void runMotor(uint8_t channelA, uint8_t channelB, int duty);

  L9110SMotorPins pins_;
  L9110SMotorChannels channels_;
  uint32_t pwmFrequency_ = 20000;
  uint8_t pwmResolutionBits_ = 10;
  int maxDuty_ = 1023;
};
