#include "WifiDebug.h"

namespace {
String bssidToString(const uint8_t* bssid) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  return String(buf);
}
}

WifiDebug::WifiDebug(bool enabled) : enabled_(enabled) {}

void WifiDebug::setEnabled(bool enabled) {
  enabled_ = enabled;
}

bool WifiDebug::isEnabled() const {
  return enabled_;
}

const char* WifiDebug::statusToString(int status) {
  switch (status) {
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_WRONG_PASSWORD: return "WL_WRONG_PASSWORD";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "WL_UNKNOWN";
  }
}

void WifiDebug::logBootDiagnostics(const char* ssid) const {
  if (!enabled_) {
    return;
  }

  Serial.println("[WIFI] Boot diagnostics");
  Serial.print("[WIFI] SDK: ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("[WIFI] Reset reason: ");
  Serial.println(ESP.getResetReason());
  Serial.print("[WIFI] Chip ID: ");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("[WIFI] Flash Chip ID: ");
  Serial.println(ESP.getFlashChipId(), HEX);
  Serial.print("[WIFI] Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(ssid);
}

void WifiDebug::logInitialStatus(int status) const {
  if (!enabled_) {
    return;
  }

  Serial.print("[WIFI] Initial status: ");
  Serial.print(status);
  Serial.print(" (");
  Serial.print(statusToString(status));
  Serial.println(")");
  WiFi.printDiag(Serial);
}

void WifiDebug::logStatusChange(int status) const {
  if (!enabled_) {
    return;
  }

  Serial.print("[WIFI] Status changed -> ");
  Serial.print(status);
  Serial.print(" (");
  Serial.print(statusToString(status));
  Serial.println(")");
}

void WifiDebug::logWaiting(int status, unsigned long elapsedMs) const {
  if (!enabled_) {
    return;
  }

  Serial.print("[WIFI] Waiting... status=");
  Serial.print(status);
  Serial.print(" (");
  Serial.print(statusToString(status));
  Serial.print(")");
  Serial.print(" elapsed_ms=");
  Serial.println(elapsedMs);
}

void WifiDebug::logReconnectAttempt(int retryCount) const {
  if (!enabled_) {
    return;
  }

  Serial.print("[WIFI] Reconnect attempt #");
  Serial.println(retryCount);
}

void WifiDebug::logConnected(unsigned long elapsedMs) const {
  if (!enabled_) {
    return;
  }

  Serial.println("[WIFI] Connected");
  Serial.print("[WIFI] Elapsed ms: ");
  Serial.println(elapsedMs);
  Serial.print("[WIFI] Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WIFI] Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("[WIFI] DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("[WIFI] Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("[WIFI] MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("[WIFI] BSSID: ");
  Serial.println(WiFi.BSSIDstr());
  Serial.print("[WIFI] Channel: ");
  Serial.println(WiFi.channel());
  Serial.print("[WIFI] RSSI: ");
  Serial.println(WiFi.RSSI());
  WiFi.printDiag(Serial);
}

bool WifiDebug::scanAndSelectTarget(const char* wantedSsid, uint8_t outBssid[6], int& outChannel, int& outRssi) const {
  outRssi = -127;
  outChannel = 0;

  // Always keep a short trace for scan visibility, even when debug is disabled.
  Serial.println("[WIFI] Scanning nearby APs...");

  int n = WiFi.scanNetworks(false, true);
  if (enabled_) {
    Serial.print("[WIFI] Scan result count=");
    Serial.println(n);
  }

  if (n <= 0) {
    if (enabled_) {
      Serial.println("[WIFI] Active scan found nothing, retrying passive scan...");
    }
    n = WiFi.scanNetworks(true, true);
    if (enabled_) {
      Serial.print("[WIFI] Passive scan result count=");
      Serial.println(n);
    }
  }

  if (n <= 0) {
    if (enabled_) {
      Serial.println("[WIFI] No AP found during scan");
    }
    Serial.println("[WIFI] Scan done: target AP not found");
    return false;
  }

  bool found = false;
  for (int i = 0; i < n; ++i) {
    const String foundSsid = WiFi.SSID(i);

    if (enabled_) {
      Serial.print("[WIFI] AP[");
      Serial.print(i);
      Serial.print("] SSID=");
      Serial.print(foundSsid);
      Serial.print(" RSSI=");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" ch=");
      Serial.print(WiFi.channel(i));
      Serial.print(" enc=");
      Serial.print(WiFi.encryptionType(i));
      Serial.print(" BSSID=");
      Serial.println(WiFi.BSSIDstr(i));
    }

    if (foundSsid == wantedSsid) {
      found = true;
      if (enabled_) {
        Serial.println("[WIFI] Target SSID matched above");
      }
      if (WiFi.RSSI(i) > outRssi) {
        outRssi = WiFi.RSSI(i);
        outChannel = WiFi.channel(i);
        uint8_t* bssidPtr = WiFi.BSSID(static_cast<uint8_t>(i));
        if (bssidPtr != nullptr) {
          for (int j = 0; j < 6; ++j) {
            outBssid[j] = bssidPtr[j];
          }
        }
      }
    }
  }

  if (!found) {
    if (enabled_) {
      Serial.print("[WIFI] Target SSID not found in scan: ");
      Serial.println(wantedSsid);
    }
    Serial.println("[WIFI] Scan done: target AP not found");
    return false;
  }

  Serial.print("[WIFI] Scan done: target AP found, RSSI=");
  Serial.print(outRssi);
  Serial.print(" ch=");
  Serial.println(outChannel);

  if (enabled_) {
    Serial.print("[WIFI] Selected target AP RSSI=");
    Serial.print(outRssi);
    Serial.print(" ch=");
    Serial.print(outChannel);
    Serial.print(" BSSID=");
    Serial.println(bssidToString(outBssid));
    if (outRssi <= -82) {
      Serial.println("[WIFI] WARNING: very weak signal (<= -82 dBm), connection may flap.");
    }
  }

  return true;
}

void WifiDebug::beginConnection(const char* ssid, const char* password, bool hasTarget, const uint8_t bssid[6], int channel) const {
  if (hasTarget && channel > 0) {
    if (enabled_) {
      Serial.print("[WIFI] begin() with locked AP channel=");
      Serial.print(channel);
      Serial.print(" BSSID=");
      Serial.println(bssidToString(bssid));
    }
    WiFi.begin(ssid, password, channel, const_cast<uint8_t*>(bssid), true);
    return;
  }

  if (enabled_) {
    Serial.println("[WIFI] begin() with SSID/pass only");
  }
  WiFi.begin(ssid, password);
}
