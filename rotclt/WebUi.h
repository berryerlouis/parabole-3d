#pragma once

#include <Arduino.h>

#include "AppState.h"

class WebUi {
public:
  static String renderRoot(const AppState& state);
  static String statusJson(const AppState& state);
};
