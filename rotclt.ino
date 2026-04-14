#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <EEPROM.h>

#include "AppState.h"
#include "LedIndicator.h"
#include "MotorDriver.h"
#include "OffsetStore.h"
#include "RotctlService.h"
#include "WifiDebug.h"
#include "WebUi.h"

// WiFi
const char* ssid = "";
const char* password = "";

constexpr uint8_t LED_PIN = 2;

ESP8266WebServer server(80);

AppState appState;
OffsetStore offsetStore(appState);
RotctlService rotctl(4533, appState);
LedIndicator led(LED_PIN, true);
// Stepper pins: AZ_STEP=D1(5), AZ_DIR=D2(4), EL_STEP=D5(14), EL_DIR=D6(12)
MotorDriver motorDriver(appState, 5, 4, 14, 12);
WifiDebug wifiDebug(true);


namespace {
bool targetApKnown = false;
uint8_t targetApBssid[6] = {0, 0, 0, 0, 0, 0};
int targetApRssi = -127;
int targetApChannel = 0;

float normalizeAz(float az) {
  while (az < 0.0f) {
    az += 360.0f;
  }
  while (az >= 360.0f) {
    az -= 360.0f;
  }
  return az;
}

float clampEl(float el) {
  return constrain(el, 0.0f, 90.0f);
}

}

void handleRoot() {
  server.send(200, "text/html", WebUi::renderRoot(appState));
}

void handleStatus() {
  server.send(200, "application/json", WebUi::statusJson(appState));
}

void handleSetOffset() {
  if (server.hasArg("az")) {
    appState.offsetAz = server.arg("az").toFloat();
  }
  if (server.hasArg("el")) {
    appState.offsetEl = server.arg("el").toFloat();
  }
  offsetStore.save();
  server.send(200, "text/plain", "OK");
}

void handleSetPark() {
  if (server.hasArg("az")) {
    appState.parkAz = normalizeAz(server.arg("az").toFloat());
  }
  if (server.hasArg("el")) {
    appState.parkEl = clampEl(server.arg("el").toFloat());
  }
  offsetStore.save();
  appState.lastCommand = "Saved park";
  server.send(200, "text/plain", "OK");
}

void handlePark() {
  appState.targetAz = normalizeAz(appState.parkAz);
  appState.targetEl = clampEl(appState.parkEl);
  appState.lastCommand = "Park";
  server.send(200, "text/plain", "OK");
}

void handleManual() {
  const String dir = server.arg("d");
  float step = 2.0f;
  if (server.hasArg("s")) {
    step = server.arg("s").toFloat();
  }
  if (step <= 0.0f) {
    step = 2.0f;
  }
  step = constrain(step, 0.1f, 30.0f);

  if (dir == "left") {
    appState.targetAz = normalizeAz(appState.targetAz - step);
  } else if (dir == "right") {
    appState.targetAz = normalizeAz(appState.targetAz + step);
  } else if (dir == "up") {
    appState.targetEl = clampEl(appState.targetEl + step);
  } else if (dir == "down") {
    appState.targetEl = clampEl(appState.targetEl - step);
  } else {
    server.send(400, "text/plain", "Invalid direction");
    return;
  }

  appState.lastCommand = "Manual " + dir;
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(64);
  Serial.println();

  offsetStore.load();

  led.begin();
  led.setMode(LedMode::WifiSearching);

  Serial.println("Started");
  wifiDebug.logBootDiagnostics(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.setOutputPower(20.5f);
  WiFi.disconnect();
  delay(300);

  wifiDebug.logInitialStatus(WiFi.status());
  targetApKnown = wifiDebug.scanAndSelectTarget(ssid, targetApBssid, targetApChannel, targetApRssi);
  wifiDebug.beginConnection(ssid, password, targetApKnown, targetApBssid, targetApChannel);

  const unsigned long connectStartMs = millis();
  unsigned long lastStatusLogMs = 0;
  unsigned long lastRetryMs = 0;
  int lastStatus = WL_IDLE_STATUS;
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    led.tick();
    delay(20);

    const unsigned long now = millis();
    const int st = WiFi.status();

    if (st != lastStatus) {
      wifiDebug.logStatusChange(st);
      lastStatus = st;
    }

    if (now - lastStatusLogMs >= 1000) {
      wifiDebug.logWaiting(st, now - connectStartMs);
      lastStatusLogMs = now;
    }

    // If stuck disconnected/connect-failed/wrong-password, retry association periodically.
    if ((st == WL_DISCONNECTED || st == WL_CONNECT_FAILED || st == WL_WRONG_PASSWORD || st == WL_NO_SSID_AVAIL) &&
        (now - lastRetryMs >= 10000)) {
      retryCount++;
      wifiDebug.logReconnectAttempt(retryCount);
      targetApKnown = wifiDebug.scanAndSelectTarget(ssid, targetApBssid, targetApChannel, targetApRssi);
      WiFi.disconnect();
      delay(50);
      wifiDebug.beginConnection(ssid, password, targetApKnown, targetApBssid, targetApChannel);
      lastRetryMs = now;
    }
  }

  led.pulse(200);
  delay(220);

  wifiDebug.logConnected(millis() - connectStartMs);

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/setoffset", handleSetOffset);
  server.on("/setpark", handleSetPark);
  server.on("/park", handlePark);
  server.on("/manual", handleManual);
  server.begin();

  rotctl.begin();

  Wire.begin();
  motorDriver.begin();
}

void loop() {
  server.handleClient();
  rotctl.process();

  motorDriver.update();

  led.setMode(appState.clientConnected ? LedMode::RotctlConnected : LedMode::Idle);
  led.tick();
}
