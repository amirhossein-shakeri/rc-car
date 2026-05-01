#pragma once

#include <Arduino.h>

class MotorDriver
{
public:
  virtual ~MotorDriver() = default;

  virtual void begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits) = 0;
  virtual void driveDuties(int leftDuty, int rightDuty) = 0;
  virtual void stop() = 0;
  virtual int maxDuty() const = 0;
  virtual const char *name() const = 0;
};
