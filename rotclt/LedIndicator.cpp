#include "LedIndicator.h"

LedIndicator::LedIndicator(uint8_t pin, uint8_t pin_motor, bool activeLow)
    : pin_(pin), pin_motor_(pin_motor), activeLow_(activeLow) {}

void LedIndicator::begin() {
  pinMode(pin_, OUTPUT);
  pinMode(pin_motor_, OUTPUT);
  writeStatusLed(false);
}

void LedIndicator::setMode(LedMode mode) {
  if (mode_ == mode) {
    return;
  }
  mode_ = mode;
  lastToggleMs_ = 0;
}

void LedIndicator::setMotorEnable(bool enabled) { writeMotorLed(enabled); }

void LedIndicator::pulse(unsigned long onMs) {
  writeStatusLed(true);
  pulseUntilMs_ = millis() + onMs;
}

void LedIndicator::tick() {
  const unsigned long now = millis();

  if (pulseUntilMs_ != 0) {
    if ((long)(now - pulseUntilMs_) >= 0) {
      pulseUntilMs_ = 0;
      writeStatusLed(false);
    }
    return;
  }

  const unsigned long interval = intervalForMode();
  if (interval == 0) {
    writeStatusLed(false);
    return;
  }

  if (now - lastToggleMs_ >= interval) {
    state_ = !state_;
    writeStatusLed(state_);
    lastToggleMs_ = now;
  }
}

void LedIndicator::writeStatusLed(bool on) {
  state_ = on;
  if (activeLow_) {
    digitalWrite(pin_, on ? LOW : HIGH);
  } else {
    digitalWrite(pin_, on ? HIGH : LOW);
  }
}

void LedIndicator::writeMotorLed(bool enabled) {
  digitalWrite(pin_motor_, enabled ? HIGH : LOW);
}

unsigned long LedIndicator::intervalForMode() const {
  switch (mode_) {
  case LedMode::WifiSearching:
    return 500;
  case LedMode::Idle:
    return 1000;
  case LedMode::RotctlConnected:
    return 80;
  case LedMode::Off:
  default:
    return 0;
  }
}
