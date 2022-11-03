#pragma once

#include <string>
#include "position.h"

struct Metric{
  std::string status;
  Position position;

  Metric(std::string status, Position position):
    status(status),
    position(position)
  {}
};
