#pragma once

#include <Arduino.h>

enum class LedMode { Off, WifiSearching, Idle, RotctlConnected };

class LedIndicator {
public:
  LedIndicator(uint8_t pin, uint8_t pin_motor, bool activeLow);

  void begin();
  void setMode(LedMode mode);
  void setMotorEnable(bool enabled);
  void tick();
  void pulse(unsigned long onMs);

private:
  void writeStatusLed(bool on);
  void writeMotorLed(bool enabled);
  unsigned long intervalForMode() const;

  uint8_t pin_;
  uint8_t pin_motor_;
  bool activeLow_;
  LedMode mode_ = LedMode::Off;
  bool state_ = false;
  unsigned long lastToggleMs_ = 0;
  unsigned long pulseUntilMs_ = 0;
};
