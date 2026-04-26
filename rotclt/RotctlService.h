#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "AppState.h"

class RotctlService {
public:
  RotctlService(uint16_t port, AppState& state);

  void begin();
  void process();

private:
  void acceptClientIfNeeded();
  void dropClient();
  void processIncomingBytes();
  void processLine(const String& line);

  void sendGetPosReply(bool extendedResponse);
  void sendOk();
  void sendError(int code);

  WiFiServer server_;
  WiFiClient client_;
  AppState& state_;
  String rxLine_;
};
