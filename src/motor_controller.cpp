#include "motor_controller.h"

#include <math.h>

MotorController::MotorController(MotorDriver &driver) : driver_(driver) {}

void MotorController::begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits)
{
  pwmFrequency_ = pwmFrequency;
  pwmResolutionBits_ = pwmResolutionBits;
  driver_.begin(pwmFrequency_, pwmResolutionBits_);
  maxDuty_ = driver_.maxDuty();

  Serial.printf(
      "[motor] Ready with %s. freq=%luHz resolution=%u bits maxDuty=%d\n",
      driver_.name(),
      pwmFrequency_,
      pwmResolutionBits_,
      maxDuty_);
}

void MotorController::setSpeedMultiplier(float multiplier)
{
  float bounded = constrain(multiplier, 0.0f, 1.0f);
  if (fabsf(bounded - speedMultiplier_) < 0.005f)
  {
    return;
  }
  speedMultiplier_ = bounded;
  Serial.printf("[motor] speedMultiplier=%.2f\n", speedMultiplier_);
}

void MotorController::setLeftToRightRatio(float ratio)
{
  float bounded = constrain(ratio, 0.2f, 2.0f);
  if (fabsf(bounded - leftToRightRatio_) < 0.005f)
  {
    return;
  }
  leftToRightRatio_ = bounded;
  Serial.printf("[motor] leftToRightRatio=%.2f\n", leftToRightRatio_);
}

void MotorController::setJoystickTuning(float deadzonePercent, float expo, float minStartLeftPercent, float minStartRightPercent)
{
  joystickDeadzonePercent_ = constrain(deadzonePercent, 0.0f, 30.0f);
  joystickExpo_ = constrain(expo, 1.0f, 3.0f);
  minStartLeftPercent_ = constrain(minStartLeftPercent, 0.0f, 70.0f);
  minStartRightPercent_ = constrain(minStartRightPercent, 0.0f, 70.0f);
  Serial.printf(
      "[motor] joystick tuning deadzone=%.1f%% expo=%.2f minStartL=%.1f%% minStartR=%.1f%%\n",
      joystickDeadzonePercent_,
      joystickExpo_,
      minStartLeftPercent_,
      minStartRightPercent_);
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

  driver_.driveDuties(leftSpeed, rightSpeed);
  lastLeftDuty_ = leftSpeed;
  lastRightDuty_ = rightSpeed;
}

void MotorController::driveJoystickPercent(int xPercent, int yPercent)
{
  const float tunedX = applyDeadzoneExpo(static_cast<float>(xPercent));
  const float tunedY = applyDeadzoneExpo(static_cast<float>(yPercent));

  // Arcade to tank mixing: Y drives forward/backward, X steers.
  int left = static_cast<int>(tunedY + tunedX);
  int right = static_cast<int>(tunedY - tunedX);

  left = constrain(left, -100, 100);
  right = constrain(right, -100, 100);
  left = applyMinStartPercent(left, minStartLeftPercent_);
  right = applyMinStartPercent(right, minStartRightPercent_);

  Serial.printf("[motor] joystick raw(x=%d%% y=%d%%) tuned -> left=%d%% right=%d%%\n", xPercent, yPercent, left, right);
  driveTankPercent(left, right);
}

void MotorController::stop()
{
  Serial.println("[motor] stop");
  driver_.stop();
  lastLeftDuty_ = 0;
  lastRightDuty_ = 0;
}

MotorDutyState MotorController::getDutyState() const
{
  MotorDutyState state;
  state.leftDuty = lastLeftDuty_;
  state.rightDuty = lastRightDuty_;
  state.maxDuty = maxDuty_;
  return state;
}

float MotorController::applyDeadzoneExpo(float inputPercent) const
{
  float bounded = constrain(inputPercent, -100.0f, 100.0f);
  float sign = bounded >= 0.0f ? 1.0f : -1.0f;
  float magnitude = fabsf(bounded);
  if (magnitude <= joystickDeadzonePercent_)
  {
    return 0.0f;
  }

  float normalized = (magnitude - joystickDeadzonePercent_) / (100.0f - joystickDeadzonePercent_);
  normalized = constrain(normalized, 0.0f, 1.0f);
  float curved = powf(normalized, joystickExpo_);
  return sign * curved * 100.0f;
}

int MotorController::applyMinStartPercent(int speedPercent, float minStartPercent) const
{
  if (speedPercent == 0)
  {
    return 0;
  }

  int sign = speedPercent > 0 ? 1 : -1;
  float magnitude = fabsf(static_cast<float>(speedPercent));
  float boosted = minStartPercent + (magnitude / 100.0f) * (100.0f - minStartPercent);
  boosted = constrain(boosted, minStartPercent, 100.0f);
  return sign * static_cast<int>(boosted);
}
