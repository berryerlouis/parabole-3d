#include "HttpHandler.h"
#include "AngleUtils.h"
#include "Logger.h"

static const char* TAG = "HTTP";

HttpHandler::HttpHandler(uint16_t port, AppState& state, OffsetStore& store, MotorDriver& motor)
  : server_(port), state_(state), store_(store), motor_(motor) {}

void HttpHandler::begin() {
  server_.on("/",          [this]() { onRoot(); });
  server_.on("/status",    [this]() { onStatus(); });
  server_.on("/setoffset", [this]() { onSetOffset(); });
  server_.on("/setpark",   [this]() { onSetPark(); });
  server_.on("/park",      [this]() { onPark(); });
  server_.on("/manual",    [this]() { onManual(); });
  server_.on("/enable",    [this]() { onEnable(); });
  server_.on("/calibrate", [this]() { onCalibrate(); });
  server_.begin();

  Logger::info(TAG, "Web server started");
}

void HttpHandler::handleClient() {
  server_.handleClient();
}

void HttpHandler::onRoot() {
  server_.send(200, "text/html", WebUi::renderRoot(state_));
}

void HttpHandler::onStatus() {
  server_.send(200, "application/json", WebUi::statusJson(state_));
}

void HttpHandler::onSetOffset() {
  if (server_.hasArg("az")) {
    state_.offsetAz = server_.arg("az").toFloat();
  }
  if (server_.hasArg("el")) {
    state_.offsetEl = server_.arg("el").toFloat();
  }
  store_.save();

  Logger::infof(TAG, "Offset set AZ=%.1f EL=%.1f", state_.offsetAz, state_.offsetEl);
  server_.send(200, "text/plain", "OK");
}

void HttpHandler::onSetPark() {
  if (server_.hasArg("az")) {
    state_.parkAz = AngleUtils::normalizeAz(server_.arg("az").toFloat());
  }
  if (server_.hasArg("el")) {
    state_.parkEl = AngleUtils::clampEl(server_.arg("el").toFloat());
  }
  store_.save();
  state_.lastCommand = "Saved park";

  Logger::infof(TAG, "Park set AZ=%.1f EL=%.1f", state_.parkAz, state_.parkEl);
  server_.send(200, "text/plain", "OK");
}

void HttpHandler::onPark() {
  state_.targetAz = AngleUtils::normalizeAz(state_.parkAz);
  state_.targetEl = AngleUtils::clampEl(state_.parkEl);
  state_.lastCommand = "Park";

  Logger::info(TAG, "Park command");
  server_.send(200, "text/plain", "OK");
}

void HttpHandler::onManual() {
  const String dir = server_.arg("d");
  float step = 2.0f;
  if (server_.hasArg("s")) {
    step = server_.arg("s").toFloat();
  }
  if (step <= 0.0f) step = 2.0f;
  step = constrain(step, 0.1f, 30.0f);

  if (dir == "left") {
    state_.targetAz = AngleUtils::normalizeAz(state_.targetAz - step);
  } else if (dir == "right") {
    state_.targetAz = AngleUtils::normalizeAz(state_.targetAz + step);
  } else if (dir == "up") {
    state_.targetEl = AngleUtils::clampEl(state_.targetEl + step);
  } else if (dir == "down") {
    state_.targetEl = AngleUtils::clampEl(state_.targetEl - step);
  } else {
    server_.send(400, "text/plain", "Invalid direction");
    return;
  }

  state_.lastCommand = "Manual " + dir;
  Logger::debugf(TAG, "Manual %s step=%.1f", dir.c_str(), step);
  server_.send(200, "text/plain", "OK");
}

void HttpHandler::onEnable() {
  if (server_.hasArg("state")) {
    state_.enable = server_.arg("state").toInt() != 0;
  }
  state_.lastCommand = state_.enable ? "Motors enabled" : "Motors disabled";

  Logger::infof(TAG, "Motors %s", state_.enable ? "enabled" : "disabled");
  server_.send(200, "text/plain", "OK");
}

void HttpHandler::onCalibrate() {
  float azStepsPerDeg = 200 * 16 * (144.0f / 17.0f) / 360.0f;
  float elStepsPerDeg = 200 * 16 * (64.0f / 21.0f) / 360.0f;

  long azSteps = static_cast<long>(AngleUtils::normalizeAz(state_.parkAz) * azStepsPerDeg);
  motor_.setCurrentPositionAz(azSteps);

  long elSteps = static_cast<long>(AngleUtils::clampEl(state_.parkEl) * elStepsPerDeg);
  motor_.setCurrentPositionEl(elSteps);

  state_.currentAz = AngleUtils::normalizeAz(state_.parkAz);
  state_.currentEl = AngleUtils::clampEl(state_.parkEl);
  state_.lastCommand = "Calibrated to park";

  Logger::info(TAG, "Calibrated to park position");
  server_.send(200, "text/plain", "OK");
}
