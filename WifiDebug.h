#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

class WifiDebug {
public:
  explicit WifiDebug(bool enabled = false);

  void setEnabled(bool enabled);
  bool isEnabled() const;

  void logBootDiagnostics(const char* ssid) const;
  void logInitialStatus(int status) const;
  void logStatusChange(int status) const;
  void logWaiting(int status, unsigned long elapsedMs) const;
  void logReconnectAttempt(int retryCount) const;
  void logConnected(unsigned long elapsedMs) const;

  bool scanAndSelectTarget(const char* wantedSsid, uint8_t outBssid[6], int& outChannel, int& outRssi) const;
  void beginConnection(const char* ssid, const char* password, bool hasTarget, const uint8_t bssid[6], int channel) const;

  static const char* statusToString(int status);

private:
  bool enabled_;
};
