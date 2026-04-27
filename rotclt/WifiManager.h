#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "LedIndicator.h"

class WifiManager {
public:
  WifiManager(const char* ssid, const char* password);

  void connect(LedIndicator& led);
  bool isConnected() const;
  IPAddress localIP() const;

private:
  void scanAndSelectTarget();
  void beginConnection();
  void waitForConnection(LedIndicator& led);

  static const char* statusToString(int status);

  const char* ssid_;
  const char* password_;

  bool targetKnown_ = false;
  uint8_t targetBssid_[6] = {};
  int targetRssi_ = -127;
  int targetChannel_ = 0;
};
