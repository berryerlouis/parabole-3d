#include "MotorDriver.h"
#include "Logger.h"

static const char* TAG = "MOTOR";

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

  Logger::info(TAG, "Motor driver initialized");
}

void MotorDriver::update() {
  if (state_.targetAz != lastTargetAz_) {
    float currentDeg = AngleUtils::normalizeAz(stepsToDegreesAz(stepperAz_.currentPosition()));
    float delta = AngleUtils::shortestAzDelta(state_.targetAz, currentDeg);
    long steps = static_cast<long>(delta * kStepsPerRotationAz / 360.0f);
    stepperAz_.moveTo(stepperAz_.currentPosition() + steps);
    lastTargetAz_ = state_.targetAz;
  }

  if (state_.targetEl != lastTargetEl_) {
    long target = static_cast<long>(AngleUtils::clampEl(state_.targetEl) * kStepsPerRotationEl / 360.0f);
    stepperEl_.moveTo(target);
    lastTargetEl_ = state_.targetEl;
  }

  const bool shouldEnable = state_.enable && (stepperAz_.distanceToGo() != 0 || stepperEl_.distanceToGo() != 0);
  setEnable(state_.enable);

  stepperAz_.run();
  stepperEl_.run();

  state_.currentAz = AngleUtils::normalizeAz(stepsToDegreesAz(stepperAz_.currentPosition()));
  state_.currentEl = AngleUtils::clampEl(stepsToDegreesEl(stepperEl_.currentPosition()));
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
  Logger::infof(TAG, "Motors %s (pin=%d)", enabled ? "enabled" : "disabled", value);
  digitalWrite(enablePin_, value);
}

float MotorDriver::stepsToDegreesAz(long steps) const {
  return steps * 360.0f / kStepsPerRotationAz;
}

float MotorDriver::stepsToDegreesEl(long steps) const {
  return steps * 360.0f / kStepsPerRotationEl;
}

long MotorDriver::degreesToStepsAz(float deg) const {
  return static_cast<long>(AngleUtils::normalizeAz(deg) * kStepsPerRotationAz / 360.0f);
}

long MotorDriver::degreesToStepsEl(float deg) const {
  return static_cast<long>(AngleUtils::clampEl(deg) * kStepsPerRotationEl / 360.0f);
}
