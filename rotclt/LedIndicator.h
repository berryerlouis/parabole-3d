#pragma once

#include <Arduino.h>

enum class LedMode {
  Off,
  WifiSearching,
  Idle,
  RotctlConnected
};

class LedIndicator {
public:
  LedIndicator(uint8_t pin, bool activeLow);

  void begin();
  void setMode(LedMode mode);
  void tick();
  void pulse(unsigned long onMs);

private:
  void write(bool on);
  unsigned long intervalForMode() const;

  uint8_t pin_;
  bool activeLow_;
  LedMode mode_ = LedMode::Off;
  bool state_ = false;
  unsigned long lastToggleMs_ = 0;
  unsigned long pulseUntilMs_ = 0;
};
