#include "OffsetStore.h"
#include "Logger.h"

#include <math.h>

static const char* TAG = "STORE";

namespace {
constexpr int kOffsetAzAddr = 0;
constexpr int kOffsetElAddr = 4;
constexpr int kParkAzAddr = 8;
constexpr int kParkElAddr = 12;
constexpr int kCurrentAzAddr = 16;
constexpr int kCurrentElAddr = 20;
}

OffsetStore::OffsetStore(AppState& state) : state_(state) {}

void OffsetStore::save() {
  EEPROM.put(kOffsetAzAddr, state_.offsetAz);
  EEPROM.put(kOffsetElAddr, state_.offsetEl);
  EEPROM.put(kParkAzAddr, state_.parkAz);
  EEPROM.put(kParkElAddr, state_.parkEl);
  EEPROM.commit();

  Logger::infof(TAG, "Saved offset AZ=%.1f EL=%.1f", state_.offsetAz, state_.offsetEl);
  Logger::infof(TAG, "Saved park AZ=%.1f EL=%.1f", state_.parkAz, state_.parkEl);
}

void OffsetStore::saveCurrentPosition(float currentAz, float currentEl) {
  EEPROM.put(kCurrentAzAddr, currentAz);
  EEPROM.put(kCurrentElAddr, currentEl);
  EEPROM.commit();
  Logger::infof(TAG, "Saved position AZ=%.1f EL=%.1f", currentAz, currentEl);
}

void OffsetStore::load() {
  EEPROM.get(kOffsetAzAddr, state_.offsetAz);
  EEPROM.get(kOffsetElAddr, state_.offsetEl);
  EEPROM.get(kParkAzAddr, state_.parkAz);
  EEPROM.get(kParkElAddr, state_.parkEl);
  EEPROM.get(kCurrentAzAddr, state_.currentAz);
  EEPROM.get(kCurrentElAddr, state_.currentEl);

  bool shouldSave = false;
  if (isnan(state_.offsetAz)) {
    state_.offsetAz = 0.0f;
    shouldSave = true;
  }
  if (isnan(state_.offsetEl)) {
    state_.offsetEl = 0.0f;
    shouldSave = true;
  }
  if (isnan(state_.parkAz)) {
    state_.parkAz = 180.0f;
    shouldSave = true;
  }
  if (isnan(state_.parkEl)) {
    state_.parkEl = 90.0f;
    shouldSave = true;
  }
  if (isnan(state_.currentAz)) {
    state_.currentAz = 0.0f;
    shouldSave = true;
  }
  if (isnan(state_.currentEl)) {
    state_.currentEl = 0.0f;
    shouldSave = true;
  }
  if (shouldSave) {
    save();
    saveCurrentPosition(state_.currentAz, state_.currentEl);
  }

  Logger::infof(TAG, "Loaded offset AZ=%.1f EL=%.1f", state_.offsetAz, state_.offsetEl);
  Logger::infof(TAG, "Loaded park AZ=%.1f EL=%.1f", state_.parkAz, state_.parkEl);
  Logger::infof(TAG, "Loaded position AZ=%.1f EL=%.1f", state_.currentAz, state_.currentEl);
}
