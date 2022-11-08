#pragma once

#include <string>

struct LogData{
  std::string message;
  std::string level;

  LogData(std::string message, std::string level):
    message(message),
    level(level)
  {}
};
