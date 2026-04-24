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
    if (config_.buzzerUsePwm)
    {
      ledcSetup(config_.buzzerPwmChannel, config_.buzzerFrequencyHz, 8);
      ledcAttachPin(config_.buzzerPin, config_.buzzerPwmChannel);
      ledcWrite(config_.buzzerPwmChannel, 0);
      Serial.printf(
          "[fx] Buzzer PWM enabled pin=%u channel=%u freq=%uHz volume=%u\n",
          config_.buzzerPin,
          config_.buzzerPwmChannel,
          config_.buzzerFrequencyHz,
          config_.buzzerVolume);
    }
    else
    {
      pinMode(config_.buzzerPin, OUTPUT);
      digitalWrite(config_.buzzerPin, LOW);
      Serial.printf("[fx] Buzzer digital mode enabled pin=%u\n", config_.buzzerPin);
    }
  }
}

void FeedbackController::playStartupSound()
{
  if (!config_.enableBuzzer)
  {
    return;
  }

  // Startup melody is queued, so startup remains responsive.
  queueNote(880, config_.buzzerVolume / 2, 70, 30);
  queueNote(1240, config_.buzzerVolume, 120, 0);
  Serial.println("[fx] Startup sound played.");
}

void FeedbackController::playWifiConnectedSound()
{
  if (!config_.enableBuzzer)
  {
    return;
  }
  queueNote(1046, config_.buzzerVolume / 2, 50, 20);
  queueNote(1396, config_.buzzerVolume, 80, 0);
  Serial.println("[fx] Wi-Fi connected sound queued.");
}

void FeedbackController::playTuneSavedSound()
{
  if (!config_.enableBuzzer)
  {
    return;
  }
  queueNote(1318, config_.buzzerVolume / 2, 45, 20);
  queueNote(1760, config_.buzzerVolume, 65, 0);
  Serial.println("[fx] Tune saved sound queued.");
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
    stopBuzzer();
    buzzerActive_ = false;
  }

  runBuzzerQueue();
}

void FeedbackController::queueNote(uint16_t frequencyHz, uint8_t volume, uint16_t durationMs, uint16_t pauseAfterMs)
{
  uint8_t nextTail = (noteTail_ + 1) % kMaxQueuedNotes;
  if (nextTail == noteHead_)
  {
    Serial.println("[fx] Note queue full, dropping note.");
    return;
  }
  noteQueue_[noteTail_].frequencyHz = frequencyHz;
  noteQueue_[noteTail_].volume = volume;
  noteQueue_[noteTail_].durationMs = durationMs;
  noteQueue_[noteTail_].pauseAfterMs = pauseAfterMs;
  noteTail_ = nextTail;
}

void FeedbackController::startQueuedNote(const BeepNote &note)
{
  if (!config_.enableBuzzer)
  {
    return;
  }
  if (config_.buzzerUsePwm)
  {
    ledcWriteTone(config_.buzzerPwmChannel, note.frequencyHz);
    ledcWrite(config_.buzzerPwmChannel, constrain(note.volume, static_cast<uint8_t>(0), static_cast<uint8_t>(255)));
  }
  else
  {
    digitalWrite(config_.buzzerPin, HIGH);
  }
}

void FeedbackController::stopBuzzer()
{
  if (!config_.enableBuzzer)
  {
    return;
  }
  if (config_.buzzerUsePwm)
  {
    ledcWrite(config_.buzzerPwmChannel, 0);
  }
  else
  {
    digitalWrite(config_.buzzerPin, LOW);
  }
}

void FeedbackController::runBuzzerQueue()
{
  if (!config_.enableBuzzer)
  {
    return;
  }

  unsigned long now = millis();
  if (notePlaying_)
  {
    if (now >= noteEndsAtMs_)
    {
      stopBuzzer();
      notePlaying_ = false;
    }
    return;
  }

  if (now < notePauseEndsAtMs_)
  {
    return;
  }

  if (noteHead_ == noteTail_)
  {
    return;
  }

  BeepNote note = noteQueue_[noteHead_];
  noteHead_ = (noteHead_ + 1) % kMaxQueuedNotes;
  startQueuedNote(note);
  notePlaying_ = true;
  noteEndsAtMs_ = now + note.durationMs;
  notePauseEndsAtMs_ = noteEndsAtMs_ + note.pauseAfterMs;
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
  queueNote(config_.buzzerFrequencyHz, config_.buzzerVolume, config_.stopBeepMs, 0);
  buzzerActive_ = false;
  buzzerOffAtMs_ = 0;
  Serial.println("[fx] stop beep");
}
