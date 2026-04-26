#include "MotorDriver.h"

MotorDriver::MotorDriver(AppState& state,
                         uint8_t azStepPin, uint8_t azDirPin,
                         uint8_t elStepPin, uint8_t elDirPin,
                         uint8_t enablePin,
                         bool enableActiveLow)
  : state_(state),
    stepperAz_(AccelStepper::DRIVER, azStepPin, azDirPin),
    stepperEl_(AccelStepper::DRIVER, elStepPin, elDirPin),
    enablePin_(enablePin),
    enableActiveLow_(enableActiveLow) {}

void MotorDriver::begin() {
  if (enablePin_ != 255) {
    pinMode(enablePin_, OUTPUT);
    setEnable(false);
  }

  float stepsPerDegAz = kStepsPerRotationAz / 360.0f;
  stepperAz_.setMaxSpeed(kAzMaxSpeedDegPerSec * stepsPerDegAz);
  stepperAz_.setAcceleration(kAzAccelDegPerSec2 * stepsPerDegAz);

  float stepsPerDegEl = kStepsPerRotationEl / 360.0f;
  stepperEl_.setMaxSpeed(kElMaxSpeedDegPerSec * stepsPerDegEl);
  stepperEl_.setAcceleration(kElAccelDegPerSec2 * stepsPerDegEl);

  stepperAz_.setCurrentPosition(degreesToStepsAz(state_.currentAz));
  stepperEl_.setCurrentPosition(degreesToStepsEl(state_.currentEl));
}

void MotorDriver::update() {
  if (state_.targetAz != lastTargetAz_) {
    float currentDeg = normalizeAz(stepsToDegreesAz(stepperAz_.currentPosition()));
    float delta = shortestAzDelta(state_.targetAz, currentDeg);
    long steps = static_cast<long>(delta * kStepsPerRotationAz / 360.0f);
    stepperAz_.moveTo(stepperAz_.currentPosition() + steps);
    lastTargetAz_ = state_.targetAz;
  }

  if (state_.targetEl != lastTargetEl_) {
    long target = static_cast<long>(clampEl(state_.targetEl) * kStepsPerRotationEl / 360.0f);
    stepperEl_.moveTo(target);
    lastTargetEl_ = state_.targetEl;
  }

  const bool shouldEnable = state_.enable && (stepperAz_.distanceToGo() != 0 || stepperEl_.distanceToGo() != 0);
  setEnable(state_.enable);

  stepperAz_.run();
  stepperEl_.run();

  state_.currentAz = normalizeAz(stepsToDegreesAz(stepperAz_.currentPosition()));
  state_.currentEl = clampEl(stepsToDegreesEl(stepperEl_.currentPosition()));
}

void MotorDriver::setCurrentPositionAz(long pos) {
  stepperAz_.setCurrentPosition(pos);
}

void MotorDriver::setCurrentPositionEl(long pos) {
  stepperEl_.setCurrentPosition(pos);
}

void MotorDriver::setEnable(bool enabled) {
  if (enablePin_ == 255) {
    return;
  }

  if (enabled == enabled_) {
    return;
  }

  enabled_ = enabled;
  const int value = (enabled ^ enableActiveLow_) ? HIGH : LOW;
  Serial.printf("Motors %s - value: %d\n", enabled ? "enabled" : "disabled", value);
  digitalWrite(enablePin_, value);
}

float MotorDriver::stepsToDegreesAz(long steps) const {
  return steps * 360.0f / kStepsPerRotationAz;
}

float MotorDriver::stepsToDegreesEl(long steps) const {
  return steps * 360.0f / kStepsPerRotationEl;
}

long MotorDriver::degreesToStepsAz(float deg) const {
  return static_cast<long>(normalizeAz(deg) * kStepsPerRotationAz / 360.0f);
}

long MotorDriver::degreesToStepsEl(float deg) const {
  return static_cast<long>(clampEl(deg) * kStepsPerRotationEl / 360.0f);
}

float MotorDriver::normalizeAz(float az) {
  while (az < 0.0f) az += 360.0f;
  while (az >= 360.0f) az -= 360.0f;
  return az;
}

float MotorDriver::clampEl(float el) {
  return constrain(el, 0.0f, 90.0f);
}

float MotorDriver::shortestAzDelta(float target, float current) {
  float d = target - current;
  while (d > 180.0f) d -= 360.0f;
  while (d < -180.0f) d += 360.0f;
  return d;
}
