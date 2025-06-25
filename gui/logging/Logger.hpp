#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>

enum class LogLevel : std::uint8_t {
  DEBUG,
  INFO,
  WARN,
  CRIT
};

enum class LogType : std::uint8_t {
  JSON,
  SIMPLE
};

using LogSettings = struct log_settings {
  LogLevel filter;
  LogType type;
};

class LogStream {
public:
  static LogSettings SETTINGS;

  LogStream(LogLevel level, const char *file, int line);
  ~LogStream();

  template <typename T> auto operator<<(const T &val) -> LogStream &
  {
    _ss << val;
    return *this;
  }

  void flush();
  static auto log(LogLevel lvl, const char *file, int line) -> LogStream;
  static void
  logger_configure(const std::string &log_path, LogLevel level, LogType type);
  static std::string cleanString(std::string str);

private:
  LogLevel _level;
  const char *_file;
  int _line;
  std::ostringstream _ss;
};

class Log : public LogStream {};

#define debug log(LogLevel::DEBUG, __FILE__, __LINE__)
#define inf log(LogLevel::INFO, __FILE__, __LINE__)
#define warn log(LogLevel::WARN, __FILE__, __LINE__)
#define failed log(LogLevel::CRIT, __FILE__, __LINE__)
