#include "feedback_controller.h"

FeedbackController::FeedbackController(const FeedbackConfig &config) : config_(config) {}

void FeedbackController::begin()
{
  if (config_.enableLed)
  {
    ledcSetup(config_.ledPwmChannel, 5000, 8);
    ledcAttachPin(config_.ledPin, config_.ledPwmChannel);
    applyLed(0);
    Serial.printf("[fx] LED feedback enabled on pin=%u channel=%u\n", config_.ledPin, config_.ledPwmChannel);
  }

  if (config_.enableBuzzer)
  {
    pinMode(config_.buzzerPin, OUTPUT);
    digitalWrite(config_.buzzerPin, LOW);
    Serial.printf("[fx] Buzzer feedback enabled on pin=%u\n", config_.buzzerPin);
  }
}

void FeedbackController::playStartupSound()
{
  if (!config_.enableBuzzer)
  {
    return;
  }

  // Startup beep sequence is intentionally blocking but only runs at boot.
  digitalWrite(config_.buzzerPin, HIGH);
  delay(45);
  digitalWrite(config_.buzzerPin, LOW);
  delay(35);
  digitalWrite(config_.buzzerPin, HIGH);
  delay(70);
  digitalWrite(config_.buzzerPin, LOW);
  delay(20);
  Serial.println("[fx] Startup sound played.");
}

void FeedbackController::updateFromMotor(const MotorDutyState &dutyState)
{
  int maxAbsDuty = max(abs(dutyState.leftDuty), abs(dutyState.rightDuty));
  bool isMoving = maxAbsDuty > 0;

  if (config_.enableLed)
  {
    ledBrightness_ = map(maxAbsDuty, 0, dutyState.maxDuty, 0, 255);
  }

  if (config_.enableBuzzer && wasMoving_ && !isMoving)
  {
    triggerStopBeep();
  }
  wasMoving_ = isMoving;
}

void FeedbackController::loop()
{
  unsigned long now = millis();

  if (config_.enableLed)
  {
    if (ledBrightness_ > 0)
    {
      applyLed(ledBrightness_);
      stopBlinkState_ = false;
      lastBlinkAtMs_ = now;
    }
    else
    {
      if (now - lastBlinkAtMs_ >= 600)
      {
        lastBlinkAtMs_ = now;
        stopBlinkState_ = !stopBlinkState_;
      }
      applyLed(stopBlinkState_ ? 10 : 0);
    }
  }

  if (config_.enableBuzzer && buzzerActive_ && now >= buzzerOffAtMs_)
  {
    digitalWrite(config_.buzzerPin, LOW);
    buzzerActive_ = false;
  }
}

void FeedbackController::applyLed(int brightness)
{
  int bounded = constrain(brightness, 0, 255);
  int duty = config_.ledActiveHigh ? bounded : (255 - bounded);
  ledcWrite(config_.ledPwmChannel, duty);
}

void FeedbackController::triggerStopBeep()
{
  if (!config_.enableBuzzer)
  {
    return;
  }
  digitalWrite(config_.buzzerPin, HIGH);
  buzzerActive_ = true;
  buzzerOffAtMs_ = millis() + config_.stopBeepMs;
  Serial.println("[fx] stop beep");
}
