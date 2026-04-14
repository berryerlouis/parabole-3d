#pragma once

#include <Arduino.h>
#include <AccelStepper.h>

#include "AppState.h"

class MotorDriver {
public:
  MotorDriver(AppState& state,
              uint8_t azStepPin, uint8_t azDirPin,
              uint8_t elStepPin, uint8_t elDirPin);

  void begin();
  void update();

private:
  static float normalizeAz(float az);
  static float clampEl(float el);
  static float shortestAzDelta(float target, float current);

  long degreesToSteps(float degrees) const;
  float stepsToDegrees(long steps) const;

  AppState& state_;
  AccelStepper stepperAz_;
  AccelStepper stepperEl_;

  float lastTargetAz_ = 0.0f;
  float lastTargetEl_ = 0.0f;

  // NEMA stepper: 200 full steps/rev, 16x microstepping = 3200 steps/rev
  static constexpr int kStepsPerRev = 200;
  static constexpr int kMicrosteps = 16;
  static constexpr long kStepsPerRotation = static_cast<long>(kStepsPerRev) * kMicrosteps;

  static constexpr float kAzMaxSpeedDegPerSec = 55.0f;
  static constexpr float kElMaxSpeedDegPerSec = 35.0f;
  static constexpr float kAzAccelDegPerSec2 = 180.0f;
  static constexpr float kElAccelDegPerSec2 = 140.0f;
};
