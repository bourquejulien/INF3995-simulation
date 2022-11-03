#pragma once

#include <string>
#include "position.h"

struct Metric{
  int status;
  Position position;

  Metric(int status, Position position):
    status(status),
    position(position)
  {}
};
