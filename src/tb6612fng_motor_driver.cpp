#include "tb6612fng_motor_driver.h"

TB6612FNGMotorDriver::TB6612FNGMotorDriver(const TB6612FNGMotorPins &pins, const TB6612FNGMotorChannels &channels)
    : pins_(pins), channels_(channels) {}

void TB6612FNGMotorDriver::begin(uint32_t pwmFrequency, uint8_t pwmResolutionBits)
{
  pwmFrequency_ = pwmFrequency;
  pwmResolutionBits_ = pwmResolutionBits;
  maxDuty_ = (1 << pwmResolutionBits_) - 1;

  Serial.println("[tb6612fng] Initializing direction-plus-PWM motor driver...");
  Serial.printf(
      "[tb6612fng] pins AIN1=%u AIN2=%u PWMA=%u BIN1=%u BIN2=%u PWMB=%u STBY=%u\n",
      pins_.ain1,
      pins_.ain2,
      pins_.pwma,
      pins_.bin1,
      pins_.bin2,
      pins_.pwmb,
      pins_.stby);
  Serial.printf("[tb6612fng] pwm channels PWMA=%u PWMB=%u\n", channels_.pwma, channels_.pwmb);

  pinMode(pins_.ain1, OUTPUT);
  pinMode(pins_.ain2, OUTPUT);
  pinMode(pins_.bin1, OUTPUT);
  pinMode(pins_.bin2, OUTPUT);
  pinMode(pins_.stby, OUTPUT);

  digitalWrite(pins_.stby, LOW);
  digitalWrite(pins_.ain1, LOW);
  digitalWrite(pins_.ain2, LOW);
  digitalWrite(pins_.bin1, LOW);
  digitalWrite(pins_.bin2, LOW);

  ledcSetup(channels_.pwma, pwmFrequency_, pwmResolutionBits_);
  ledcSetup(channels_.pwmb, pwmFrequency_, pwmResolutionBits_);
  ledcAttachPin(pins_.pwma, channels_.pwma);
  ledcAttachPin(pins_.pwmb, channels_.pwmb);

  stop();
}

void TB6612FNGMotorDriver::driveDuties(int leftDuty, int rightDuty)
{
  if (leftDuty == 0 && rightDuty == 0)
  {
    stop();
    return;
  }

  digitalWrite(pins_.stby, HIGH);
  runMotor(pins_.ain1, pins_.ain2, channels_.pwma, leftDuty);
  runMotor(pins_.bin1, pins_.bin2, channels_.pwmb, rightDuty);
}

void TB6612FNGMotorDriver::stop()
{
  runMotor(pins_.ain1, pins_.ain2, channels_.pwma, 0);
  runMotor(pins_.bin1, pins_.bin2, channels_.pwmb, 0);
  digitalWrite(pins_.stby, LOW);
}

int TB6612FNGMotorDriver::maxDuty() const
{
  return maxDuty_;
}

const char *TB6612FNGMotorDriver::name() const
{
  return "TB6612FNG";
}

void TB6612FNGMotorDriver::runMotor(uint8_t in1Pin, uint8_t in2Pin, uint8_t pwmChannel, int duty)
{
  const int bounded = constrain(duty, -maxDuty_, maxDuty_);
  if (bounded > 0)
  {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    ledcWrite(pwmChannel, bounded);
    return;
  }

  if (bounded < 0)
  {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    ledcWrite(pwmChannel, -bounded);
    return;
  }

  ledcWrite(pwmChannel, 0);
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
}
