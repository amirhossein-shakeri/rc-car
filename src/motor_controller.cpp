#include "motor_controller.h"

MotorController::MotorController(const MotorPins &pins, const MotorChannels &channels) : pins_(pins), channels_(channels) {}

void MotorController::begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits)
{
  pwmFrequency_ = pwmFrequency;
  pwmResolutionBits_ = pwmResolutionBits;
  maxDuty_ = (1 << pwmResolutionBits_) - 1;

  Serial.println("[motor] Initializing PWM channels...");
  ledcSetup(channels_.leftA, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.leftB, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.rightA, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.rightB, pwmFrequency_, pwmResolutionBits_);

  ledcAttachPin(pins_.leftA, channels_.leftA);
  ledcAttachPin(pins_.leftB, channels_.leftB);
  ledcAttachPin(pins_.rightA, channels_.rightA);
  ledcAttachPin(pins_.rightB, channels_.rightB);

  stop();
  Serial.printf("[motor] Ready. freq=%luHz resolution=%u bits maxDuty=%d\n", pwmFrequency_, pwmResolutionBits_, maxDuty_);
}

void MotorController::setSpeedMultiplier(float multiplier)
{
  speedMultiplier_ = constrain(multiplier, 0.0f, 1.0f);
  Serial.printf("[motor] speedMultiplier=%.2f\n", speedMultiplier_);
}

void MotorController::setLeftToRightRatio(float ratio)
{
  leftToRightRatio_ = constrain(ratio, 0.2f, 2.0f);
  Serial.printf("[motor] leftToRightRatio=%.2f\n", leftToRightRatio_);
}

void MotorController::driveTankPercent(int leftPercent, int rightPercent)
{
  const int boundedLeft = constrain(leftPercent, -100, 100);
  const int boundedRight = constrain(rightPercent, -100, 100);

  int leftSpeed = static_cast<int>((boundedLeft / 100.0f) * maxDuty_ * speedMultiplier_);
  int rightSpeed = static_cast<int>((boundedRight / 100.0f) * maxDuty_ * speedMultiplier_ * leftToRightRatio_);

  leftSpeed = constrain(leftSpeed, -maxDuty_, maxDuty_);
  rightSpeed = constrain(rightSpeed, -maxDuty_, maxDuty_);

  Serial.printf(
      "[motor] drive left=%d%% right=%d%% -> dutyL=%d dutyR=%d mult=%.2f ratio=%.2f\n",
      boundedLeft,
      boundedRight,
      leftSpeed,
      rightSpeed,
      speedMultiplier_,
      leftToRightRatio_);

  runMotor(channels_.leftA, channels_.leftB, leftSpeed);
  runMotor(channels_.rightA, channels_.rightB, rightSpeed);
}

void MotorController::stop()
{
  Serial.println("[motor] stop");
  runMotor(channels_.leftA, channels_.leftB, 0);
  runMotor(channels_.rightA, channels_.rightB, 0);
}

void MotorController::runMotor(uint8_t channelA, uint8_t channelB, int speed)
{
  int bounded = constrain(speed, -maxDuty_, maxDuty_);
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
