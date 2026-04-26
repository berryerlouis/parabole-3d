#pragma once

#include <EEPROM.h>
#include "AppState.h"

class OffsetStore {
public:
  explicit OffsetStore(AppState& state);

  void save();
  void load();
  void saveCurrentPosition(float currentAz, float currentEl);

private:
  AppState& state_;
};
