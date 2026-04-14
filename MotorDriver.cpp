#include "MotorDriver.h"

MotorDriver::MotorDriver(AppState& state,
                         uint8_t azStepPin, uint8_t azDirPin,
                         uint8_t elStepPin, uint8_t elDirPin)
  : state_(state),
    stepperAz_(AccelStepper::DRIVER, azStepPin, azDirPin),
    stepperEl_(AccelStepper::DRIVER, elStepPin, elDirPin) {}

void MotorDriver::begin() {
  float stepsPerDeg = kStepsPerRotation / 360.0f;

  stepperAz_.setMaxSpeed(kAzMaxSpeedDegPerSec * stepsPerDeg);
  stepperAz_.setAcceleration(kAzAccelDegPerSec2 * stepsPerDeg);

  stepperEl_.setMaxSpeed(kElMaxSpeedDegPerSec * stepsPerDeg);
  stepperEl_.setAcceleration(kElAccelDegPerSec2 * stepsPerDeg);

  stepperAz_.setCurrentPosition(0);
  stepperEl_.setCurrentPosition(0);
}

void MotorDriver::update() {
  if (state_.targetAz != lastTargetAz_) {
    float currentDeg = normalizeAz(stepsToDegrees(stepperAz_.currentPosition()));
    float delta = shortestAzDelta(state_.targetAz, currentDeg);
    stepperAz_.moveTo(stepperAz_.currentPosition() + degreesToSteps(delta));
    lastTargetAz_ = state_.targetAz;
  }

  if (state_.targetEl != lastTargetEl_) {
    long target = degreesToSteps(clampEl(state_.targetEl));
    stepperEl_.moveTo(target);
    lastTargetEl_ = state_.targetEl;
  }

  stepperAz_.run();
  stepperEl_.run();

  state_.currentAz = normalizeAz(stepsToDegrees(stepperAz_.currentPosition()));
  state_.currentEl = clampEl(stepsToDegrees(stepperEl_.currentPosition()));
}

long MotorDriver::degreesToSteps(float degrees) const {
  return static_cast<long>(degrees * kStepsPerRotation / 360.0f);
}

float MotorDriver::stepsToDegrees(long steps) const {
  return steps * 360.0f / kStepsPerRotation;
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
