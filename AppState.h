#pragma once

#include <Arduino.h>

struct AppState {
  float currentAz = 0.0f;
  float currentEl = 0.0f;
  float targetAz = 0.0f;
  float targetEl = 0.0f;
  float offsetAz = 0.0f;
  float offsetEl = 0.0f;
  float parkAz = 180.0f;
  float parkEl = 0.0f;

  String lastCommand = "None";
  bool clientConnected = false;
};
