#pragma once

#include <Arduino.h>
#include <AccelStepper.h>

#include "AppState.h"

class MotorDriver {
public:
  MotorDriver(AppState& state,
              uint8_t azStepPin, uint8_t azDirPin,
              uint8_t elStepPin, uint8_t elDirPin,
              uint8_t enablePin = 255,
              bool enableActiveLow = true);

  void begin();
  void update();
  void setCurrentPositionAz(long pos);
  void setCurrentPositionEl(long pos);

private:
  static float normalizeAz(float az);
  static float clampEl(float el);
  static float shortestAzDelta(float target, float current);

  float stepsToDegreesAz(long steps) const;
  float stepsToDegreesEl(long steps) const;
  long degreesToStepsAz(float deg) const;
  long degreesToStepsEl(float deg) const;
  void setEnable(bool enabled);

  AppState& state_;
  AccelStepper stepperAz_;
  AccelStepper stepperEl_;

  float lastTargetAz_ = 0.0f;
  float lastTargetEl_ = 0.0f;

  uint8_t enablePin_ = 255;
  bool enableActiveLow_ = true;
  bool enabled_ = true;

  // NEMA stepper: 200 full steps/rev, 16x microstepping = 3200 steps/rev
  static constexpr int kStepsPerRev = 200;
  static constexpr int kMicrosteps = 16;

  // Gear ratios: motor rev per output rev
  static constexpr float kAzGearRatio = 144.0f / 17.0f;
  static constexpr float kElGearRatio = 64.0f / 21.0f;

  // Effective steps per output rotation
  static constexpr long kStepsPerRotationAz = static_cast<long>(kStepsPerRev * kMicrosteps * kAzGearRatio);
  static constexpr long kStepsPerRotationEl = static_cast<long>(kStepsPerRev * kMicrosteps * kElGearRatio);

  static constexpr float kAzMaxSpeedDegPerSec = 55.0f;
  static constexpr float kElMaxSpeedDegPerSec = 35.0f;
  static constexpr float kAzAccelDegPerSec2 = 180.0f;
  static constexpr float kElAccelDegPerSec2 = 140.0f;
};
