#include "l9110s_motor_driver.h"

L9110SMotorDriver::L9110SMotorDriver(const L9110SMotorPins &pins, const L9110SMotorChannels &channels)
    : pins_(pins), channels_(channels) {}

void L9110SMotorDriver::begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits)
{
  pwmFrequency_ = pwmFrequency;
  pwmResolutionBits_ = pwmResolutionBits;
  maxDuty_ = (1 << pwmResolutionBits_) - 1;

  Serial.println("[l9110s] Initializing four-pin PWM H-bridge driver...");
  Serial.printf(
      "[l9110s] pins L(A,B)=(%u,%u) R(A,B)=(%u,%u)\n",
      pins_.leftA,
      pins_.leftB,
      pins_.rightA,
      pins_.rightB);
  Serial.printf(
      "[l9110s] channels L(A,B)=(%u,%u) R(A,B)=(%u,%u)\n",
      channels_.leftA,
      channels_.leftB,
      channels_.rightA,
      channels_.rightB);

  ledcSetup(channels_.leftA, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.leftB, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.rightA, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.rightB, pwmFrequency_, pwmResolutionBits_);

  ledcAttachPin(pins_.leftA, channels_.leftA);
  ledcAttachPin(pins_.leftB, channels_.leftB);
  ledcAttachPin(pins_.rightA, channels_.rightA);
  ledcAttachPin(pins_.rightB, channels_.rightB);

  stop();
}

void L9110SMotorDriver::driveDuties(int leftDuty, int rightDuty)
{
  runMotor(channels_.leftA, channels_.leftB, leftDuty);
  runMotor(channels_.rightA, channels_.rightB, rightDuty);
}

void L9110SMotorDriver::stop()
{
  driveDuties(0, 0);
}

int L9110SMotorDriver::maxDuty() const
{
  return maxDuty_;
}

const char *L9110SMotorDriver::name() const
{
  return "L9110S";
}

void L9110SMotorDriver::runMotor(uint8_t channelA, uint8_t channelB, int duty)
{
  const int bounded = constrain(duty, -maxDuty_, maxDuty_);
  if (bounded > 0)
  {
    ledcWrite(channelA, bounded);
    ledcWrite(channelB, 0);
    return;
  }

  if (bounded < 0)
  {
    ledcWrite(channelA, 0);
    ledcWrite(channelB, -bounded);
    return;
  }

  ledcWrite(channelA, 0);
  ledcWrite(channelB, 0);
}
