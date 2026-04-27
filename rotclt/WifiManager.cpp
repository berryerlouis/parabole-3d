#include "WifiManager.h"
#include "Logger.h"

static const char* TAG = "WIFI";

namespace {
String bssidToString(const uint8_t* bssid) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  return String(buf);
}
}

WifiManager::WifiManager(const char* ssid, const char* password)
  : ssid_(ssid), password_(password) {}

void WifiManager::connect(LedIndicator& led) {
  Logger::info(TAG, "Initializing WiFi");

  Logger::debugf(TAG, "SDK: %s", ESP.getSdkVersion());
  Logger::debugf(TAG, "Reset reason: %s", ESP.getResetReason().c_str());
  Logger::debugf(TAG, "Chip ID: %08X", ESP.getChipId());
  Logger::debugf(TAG, "Free heap: %u", ESP.getFreeHeap());
  Logger::infof(TAG, "Connecting to SSID: %s", ssid_);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.setOutputPower(20.5f);
  WiFi.disconnect();
  delay(300);

  Logger::debugf(TAG, "Initial status: %d (%s)", WiFi.status(), statusToString(WiFi.status()));

  scanAndSelectTarget();
  beginConnection();
  waitForConnection(led);

  led.pulse(200);
  delay(220);

  Logger::infof(TAG, "Connected - IP: %s", WiFi.localIP().toString().c_str());
  Logger::debugf(TAG, "Gateway: %s", WiFi.gatewayIP().toString().c_str());
  Logger::debugf(TAG, "MAC: %s", WiFi.macAddress().c_str());
  Logger::debugf(TAG, "BSSID: %s", WiFi.BSSIDstr().c_str());
  Logger::debugf(TAG, "Channel: %d  RSSI: %d", WiFi.channel(), WiFi.RSSI());
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress WifiManager::localIP() const {
  return WiFi.localIP();
}

void WifiManager::scanAndSelectTarget() {
  targetRssi_ = -127;
  targetChannel_ = 0;
  targetKnown_ = false;

  Logger::info(TAG, "Scanning nearby APs...");

  int n = WiFi.scanNetworks(false, true);
  Logger::debugf(TAG, "Scan result count: %d", n);

  if (n <= 0) {
    Logger::debug(TAG, "Active scan found nothing, retrying passive...");
    n = WiFi.scanNetworks(true, true);
    Logger::debugf(TAG, "Passive scan result count: %d", n);
  }

  if (n <= 0) {
    Logger::warn(TAG, "No AP found during scan");
    return;
  }

  for (int i = 0; i < n; ++i) {
    const String foundSsid = WiFi.SSID(i);
    Logger::debugf(TAG, "AP[%d] SSID=%s RSSI=%d ch=%d",
                   i, foundSsid.c_str(), WiFi.RSSI(i), WiFi.channel(i));

    if (foundSsid == ssid_) {
      targetKnown_ = true;
      if (WiFi.RSSI(i) > targetRssi_) {
        targetRssi_ = WiFi.RSSI(i);
        targetChannel_ = WiFi.channel(i);
        uint8_t* bssidPtr = WiFi.BSSID(static_cast<uint8_t>(i));
        if (bssidPtr) {
          memcpy(targetBssid_, bssidPtr, 6);
        }
      }
    }
  }

  if (!targetKnown_) {
    Logger::warnf(TAG, "Target SSID not found: %s", ssid_);
    return;
  }

  Logger::infof(TAG, "Target AP found, RSSI=%d ch=%d BSSID=%s",
                targetRssi_, targetChannel_, bssidToString(targetBssid_).c_str());

  if (targetRssi_ <= -82) {
    Logger::warn(TAG, "Very weak signal (<= -82 dBm), connection may flap");
  }
}

void WifiManager::beginConnection() {
  if (targetKnown_ && targetChannel_ > 0) {
    Logger::debugf(TAG, "begin() with locked AP ch=%d BSSID=%s",
                   targetChannel_, bssidToString(targetBssid_).c_str());
    WiFi.begin(ssid_, password_, targetChannel_, targetBssid_, true);
  } else {
    Logger::debug(TAG, "begin() with SSID/pass only");
    WiFi.begin(ssid_, password_);
  }
}

void WifiManager::waitForConnection(LedIndicator& led) {
  const unsigned long startMs = millis();
  unsigned long lastLogMs = 0;
  unsigned long lastRetryMs = 0;
  int lastStatus = WL_IDLE_STATUS;
  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED) {
    led.tick();
    delay(20);

    const unsigned long now = millis();
    const int st = WiFi.status();

    if (st != lastStatus) {
      Logger::debugf(TAG, "Status -> %d (%s)", st, statusToString(st));
      lastStatus = st;
    }

    if (now - lastLogMs >= 1000) {
      Logger::debugf(TAG, "Waiting... status=%d elapsed=%lums", st, now - startMs);
      lastLogMs = now;
    }

    if ((st == WL_DISCONNECTED || st == WL_CONNECT_FAILED ||
         st == WL_WRONG_PASSWORD || st == WL_NO_SSID_AVAIL) &&
        (now - lastRetryMs >= 10000)) {
      retryCount++;
      Logger::warnf(TAG, "Reconnect attempt #%d", retryCount);
      scanAndSelectTarget();
      WiFi.disconnect();
      delay(50);
      beginConnection();
      lastRetryMs = now;
    }
  }
}

const char* WifiManager::statusToString(int status) {
  switch (status) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID";
    case WL_SCAN_COMPLETED:  return "SCAN_DONE";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAIL";
    case WL_CONNECTION_LOST: return "CONN_LOST";
    case WL_WRONG_PASSWORD:  return "WRONG_PASS";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}
