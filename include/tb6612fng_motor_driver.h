#pragma once

#include <Arduino.h>

#include "motor_driver.h"

struct TB6612FNGMotorPins
{
  uint8_t ain1;
  uint8_t ain2;
  uint8_t pwma;
  uint8_t bin1;
  uint8_t bin2;
  uint8_t pwmb;
  uint8_t stby;
};

struct TB6612FNGMotorChannels
{
  uint8_t pwma;
  uint8_t pwmb;
};

class TB6612FNGMotorDriver : public MotorDriver
{
public:
  TB6612FNGMotorDriver(const TB6612FNGMotorPins &pins, const TB6612FNGMotorChannels &channels);

  void begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits) override;
  void driveDuties(int leftDuty, int rightDuty) override;
  void stop() override;
  int maxDuty() const override;
  const char *name() const override;

private:
  void runMotor(uint8_t in1Pin, uint8_t in2Pin, uint8_t pwmChannel, int duty);

  TB6612FNGMotorPins pins_;
  TB6612FNGMotorChannels channels_;
  uint32_t pwmFrequency_ = 20000;
  uint8_t pwmResolutionBits_ = 10;
  int maxDuty_ = 1023;
};
