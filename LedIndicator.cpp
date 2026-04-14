#include "LedIndicator.h"

LedIndicator::LedIndicator(uint8_t pin, bool activeLow)
  : pin_(pin), activeLow_(activeLow) {}

void LedIndicator::begin() {
  pinMode(pin_, OUTPUT);
  write(false);
}

void LedIndicator::setMode(LedMode mode) {
  if (mode_ == mode) {
    return;
  }
  mode_ = mode;
  lastToggleMs_ = 0;
}

void LedIndicator::pulse(unsigned long onMs) {
  write(true);
  pulseUntilMs_ = millis() + onMs;
}

void LedIndicator::tick() {
  const unsigned long now = millis();

  if (pulseUntilMs_ != 0) {
    if ((long)(now - pulseUntilMs_) >= 0) {
      pulseUntilMs_ = 0;
      write(false);
    }
    return;
  }

  const unsigned long interval = intervalForMode();
  if (interval == 0) {
    write(false);
    return;
  }

  if (now - lastToggleMs_ >= interval) {
    state_ = !state_;
    write(state_);
    lastToggleMs_ = now;
  }
}

void LedIndicator::write(bool on) {
  state_ = on;
  if (activeLow_) {
    digitalWrite(pin_, on ? LOW : HIGH);
  } else {
    digitalWrite(pin_, on ? HIGH : LOW);
  }
}

unsigned long LedIndicator::intervalForMode() const {
  switch (mode_) {
    case LedMode::WifiSearching:
      return 400;
    case LedMode::Idle:
      return 1000;
    case LedMode::RotctlConnected:
      return 80;
    case LedMode::Off:
    default:
      return 0;
  }
}
