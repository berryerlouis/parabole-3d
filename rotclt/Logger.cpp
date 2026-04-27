#include "Logger.h"

LogLevel Logger::level_ = LogLevel::INFO;

void Logger::begin(unsigned long baud) {
  Serial.begin(baud);
  Serial.println();
}

void Logger::setLevel(LogLevel level) {
  level_ = level;
}

LogLevel Logger::level() {
  return level_;
}

void Logger::debug(const char* tag, const String& msg) {
  if (level_ > LogLevel::DEBUG) return;
  log(LogLevel::DEBUG, tag, msg);
}

void Logger::info(const char* tag, const String& msg) {
  if (level_ > LogLevel::INFO) return;
  log(LogLevel::INFO, tag, msg);
}

void Logger::warn(const char* tag, const String& msg) {
  if (level_ > LogLevel::WARN) return;
  log(LogLevel::WARN, tag, msg);
}

void Logger::error(const char* tag, const String& msg) {
  log(LogLevel::ERROR, tag, msg);
}

void Logger::log(LogLevel lvl, const char* tag, const String& msg) {
  Serial.print('[');
  Serial.print(levelStr(lvl));
  Serial.print("][");
  Serial.print(tag);
  Serial.print("] ");
  Serial.println(msg);
}

const char* Logger::levelStr(LogLevel lvl) {
  switch (lvl) {
    case LogLevel::DEBUG: return "DBG";
    case LogLevel::INFO:  return "INF";
    case LogLevel::WARN:  return "WRN";
    case LogLevel::ERROR: return "ERR";
    default:              return "???";
  }
}
