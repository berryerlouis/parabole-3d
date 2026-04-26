#include "OffsetStore.h"

#include <math.h>

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

  Serial.println("Saved offset AZ:" + String(state_.offsetAz));
  Serial.println("Saved offset EL:" + String(state_.offsetEl));
  Serial.println("Saved park AZ:" + String(state_.parkAz));
  Serial.println("Saved park EL:" + String(state_.parkEl));
}

void OffsetStore::saveCurrentPosition(float currentAz, float currentEl) {
  EEPROM.put(kCurrentAzAddr, currentAz);
  EEPROM.put(kCurrentElAddr, currentEl);
  EEPROM.commit();
  Serial.println("Saved current AZ:" + String(currentAz));
  Serial.println("Saved current EL:" + String(currentEl));
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

  Serial.println("Loaded offset AZ:" + String(state_.offsetAz));
  Serial.println("Loaded offset EL:" + String(state_.offsetEl));
  Serial.println("Loaded park AZ:" + String(state_.parkAz));
  Serial.println("Loaded park EL:" + String(state_.parkEl));
  Serial.println("Loaded current AZ:" + String(state_.currentAz));
  Serial.println("Loaded current EL:" + String(state_.currentEl));
}
