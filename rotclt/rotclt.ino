#include <Wire.h>
#include <EEPROM.h>

#include "AppState.h"
#include "HttpHandler.h"
#include "LedIndicator.h"
#include "Logger.h"
#include "MotorDriver.h"
#include "OffsetStore.h"
#include "RotctlService.h"
#include "WifiManager.h"
#include "WifiEnv.h"

// ── Configuration ──────────────────────────────────────────────

const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

constexpr uint8_t LED_PIN    = 2;
constexpr uint8_t ENABLE_PIN = 5;   // D1
constexpr uint8_t AZ_STEP    = 12;  // D6
constexpr uint8_t AZ_DIR     = 13;  // D7
constexpr uint8_t EL_STEP    = 14;  // D5
constexpr uint8_t EL_DIR     = 15;  // D8

// ── Global objects ─────────────────────────────────────────────

AppState      appState;
OffsetStore   offsetStore(appState);
MotorDriver   motorDriver(appState, AZ_STEP, AZ_DIR, EL_STEP, EL_DIR, ENABLE_PIN, true);
LedIndicator  led(LED_PIN, true);
WifiManager   wifi(ssid, password);
HttpHandler   http(80, appState, offsetStore, motorDriver);
RotctlService rotctl(4533, appState);

// ── Position auto-save state ───────────────────────────────────

static float         lastSavedAz        = 0.0f;
static float         lastSavedEl        = 0.0f;
static unsigned long lastPositionSaveMs = 0;
static int           incomingByte       = 0;
// ── Setup & Loop ───────────────────────────────────────────────

void setup() {
  Logger::begin(115200);
  EEPROM.begin(64);

  offsetStore.load();
  lastSavedAz = appState.currentAz;
  lastSavedEl = appState.currentEl;
  lastPositionSaveMs = millis();

  led.begin();
  led.setMode(LedMode::WifiSearching);

  Logger::info("APP", "Starting...");

  wifi.connect(led);

  http.begin();
  rotctl.begin();

  Wire.begin();
  motorDriver.begin();

  Logger::info("APP", "Ready");
}

bool clientRtlCtlConnected_PreviousState = false;

void loop() {

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (incomingByte == 'r') {
      Logger::info("APP", "Restarting on demand...");
      ESP.restart();
    }
  }

  http.handleClient();
  rotctl.process();

  // Enable motors only when a rotctl client is connecting or when is disconnectiong
  // to prevent unwanted movement when not in use
  if (clientRtlCtlConnected_PreviousState != appState.clientConnected) {
    clientRtlCtlConnected_PreviousState = appState.clientConnected;
    if (appState.clientConnected)
    {
      appState.enable = true;
    } else {
      appState.enable = false;
    } 
  }

  motorDriver.update();

  // Periodic position save to EEPROM
  const unsigned long now = millis();
  float deltaAz = abs(appState.currentAz - lastSavedAz);
  float deltaEl = abs(appState.currentEl - lastSavedEl);

  if ((now - lastPositionSaveMs >= 5000 && (deltaAz >= 0.1f || deltaEl >= 0.1f))
      || lastPositionSaveMs == 0) {
    offsetStore.saveCurrentPosition(appState.currentAz, appState.currentEl);
    lastSavedAz = appState.currentAz;
    lastSavedEl = appState.currentEl;
    lastPositionSaveMs = now;
  }

  led.setMode(appState.clientConnected ? LedMode::RotctlConnected : LedMode::Idle);
  led.tick();
}
