#pragma once

#include <ESP8266WebServer.h>

#include "AppState.h"
#include "MotorDriver.h"
#include "OffsetStore.h"
#include "WebUi.h"

class HttpHandler {
public:
  HttpHandler(uint16_t port, AppState& state, OffsetStore& store, MotorDriver& motor);

  void begin();
  void handleClient();

private:
  void onRoot();
  void onStatus();
  void onSetOffset();
  void onSetPark();
  void onPark();
  void onManual();
  void onEnable();
  void onCalibrate();

  ESP8266WebServer server_;
  AppState& state_;
  OffsetStore& store_;
  MotorDriver& motor_;
};
