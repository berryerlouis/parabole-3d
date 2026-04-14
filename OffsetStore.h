#pragma once

#include <EEPROM.h>
#include "AppState.h"

class OffsetStore {
public:
  explicit OffsetStore(AppState& state);

  void save();
  void load();

private:
  AppState& state_;
};
