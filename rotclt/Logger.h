#pragma once

#include <Arduino.h>

enum class LogLevel : uint8_t {
  DEBUG,
  INFO,
  WARN,
  ERROR
};

class Logger {
public:
  static void begin(unsigned long baud = 115200);
  static void setLevel(LogLevel level);
  static LogLevel level();

  static void debug(const char* tag, const String& msg);
  static void info(const char* tag, const String& msg);
  static void warn(const char* tag, const String& msg);
  static void error(const char* tag, const String& msg);

  template <typename... Args>
  static void debugf(const char* tag, const char* fmt, Args... args);

  template <typename... Args>
  static void infof(const char* tag, const char* fmt, Args... args);

  template <typename... Args>
  static void warnf(const char* tag, const char* fmt, Args... args);

  template <typename... Args>
  static void errorf(const char* tag, const char* fmt, Args... args);

private:
  static void log(LogLevel lvl, const char* tag, const String& msg);
  static const char* levelStr(LogLevel lvl);

  static LogLevel level_;
};

// Template implementations

template <typename... Args>
void Logger::debugf(const char* tag, const char* fmt, Args... args) {
  if (level_ > LogLevel::DEBUG) return;
  char buf[192];
  snprintf(buf, sizeof(buf), fmt, args...);
  log(LogLevel::DEBUG, tag, buf);
}

template <typename... Args>
void Logger::infof(const char* tag, const char* fmt, Args... args) {
  if (level_ > LogLevel::INFO) return;
  char buf[192];
  snprintf(buf, sizeof(buf), fmt, args...);
  log(LogLevel::INFO, tag, buf);
}

template <typename... Args>
void Logger::warnf(const char* tag, const char* fmt, Args... args) {
  if (level_ > LogLevel::WARN) return;
  char buf[192];
  snprintf(buf, sizeof(buf), fmt, args...);
  log(LogLevel::WARN, tag, buf);
}

template <typename... Args>
void Logger::errorf(const char* tag, const char* fmt, Args... args) {
  char buf[192];
  snprintf(buf, sizeof(buf), fmt, args...);
  log(LogLevel::ERROR, tag, buf);
}
