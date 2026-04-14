#include "RotctlService.h"

namespace {
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
  if (el < 0.0f) {
    return 0.0f;
  }
  if (el > 90.0f) {
    return 90.0f;
  }
  return el;
}
}

RotctlService::RotctlService(uint16_t port, AppState& state)
  : server_(port), state_(state) {}

void RotctlService::begin() {
  server_.begin();
}

void RotctlService::process() {
  acceptClientIfNeeded();

  if (state_.clientConnected && !client_.connected()) {
    dropClient();
    return;
  }

  if (state_.clientConnected) {
    processIncomingBytes();
  }
}

void RotctlService::acceptClientIfNeeded() {
  if (state_.clientConnected) {
    return;
  }

  WiFiClient newClient = server_.available();
  if (!newClient) {
    return;
  }

  client_ = newClient;
  client_.setTimeout(15);
  state_.clientConnected = true;
  state_.lastCommand = "Client connected";
  Serial.println("rotctl client connected");
}

void RotctlService::dropClient() {
  state_.clientConnected = false;
  state_.lastCommand = "Client disconnected";
  rxLine_ = "";
  client_.stop();
  Serial.println("rotctl client disconnected");
}

void RotctlService::processIncomingBytes() {
  while (client_.available()) {
    char c = static_cast<char>(client_.read());

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      processLine(rxLine_);
      rxLine_ = "";
      continue;
    }

    rxLine_ += c;

    if (rxLine_.length() > 120) {
      state_.lastCommand = "Line too long";
      sendError(-22);
      rxLine_ = "";
    }
  }
}

void RotctlService::processLine(const String& line) {
  String cmd = line;
  cmd.trim();

  if (cmd.length() == 0) {
    return;
  }

  state_.lastCommand = cmd;
  Serial.println("CMD: " + cmd);

  bool extendedResponse = false;
  if (cmd.startsWith("+")) {
    extendedResponse = true;
    cmd.remove(0, 1);
    cmd.trim();
  }

  if (cmd == "p" || cmd == "\\get_pos") {
    sendGetPosReply(extendedResponse);
    return;
  }

  if (cmd.startsWith("P ") || cmd.startsWith("\\set_pos ")) {
    float az = 0.0f;
    float el = 0.0f;

    int parsed = 0;
    if (cmd.startsWith("P ")) {
      parsed = sscanf(cmd.c_str(), "P %f %f", &az, &el);
    } else {
      parsed = sscanf(cmd.c_str(), "\\set_pos %f %f", &az, &el);
    }

    if (parsed == 2) {
      state_.targetAz = normalizeAz(az);
      state_.targetEl = clampEl(el);
      if (extendedResponse) {
        client_.print("set_pos: ");
        client_.print(String(state_.targetAz, 6));
        client_.print(" ");
        client_.print(String(state_.targetEl, 6));
        client_.print("\n");
      }
      sendOk();
    } else {
      sendError(-22);
    }
    return;
  }

  if (cmd == "_") {
    client_.print("rotclt-esp8266\n");
    return;
  }

  if (cmd == "q") {
    sendOk();
    dropClient();
    return;
  }

  sendError(-11);
}

void RotctlService::sendGetPosReply(bool extendedResponse) {
  if (extendedResponse) {
    client_.print("get_pos:\n");
    client_.print("Azimuth: ");
    client_.print(String(state_.currentAz, 6));
    client_.print("\n");
    client_.print("Elevation: ");
    client_.print(String(state_.currentEl, 6));
    client_.print("\n");
    client_.print("RPRT 0\n");
    return;
  }

  // SatDump v1.2.2 does one recv() then sscanf("%f\n%f", ...), so emit both lines at once.
  char reply[64];
  snprintf(reply, sizeof(reply), "%.6f\n%.6f\n", state_.currentAz, state_.currentEl);
  client_.print(reply);
}

void RotctlService::sendOk() {
  client_.print("RPRT 0\n");
}

void RotctlService::sendError(int code) {
  client_.print("RPRT ");
  client_.print(code);
  client_.print("\n");
}
