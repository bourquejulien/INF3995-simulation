#pragma once

#include <string>
#include "position.h"

struct Metric{
  int status;
  Position position;
  float battery_level;

  Metric(int status, Position position, float battery_level):
    status(status),
    position(position),
    battery_level(battery_level)
  {}
};
