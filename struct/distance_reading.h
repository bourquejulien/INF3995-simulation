#pragma once

#include "position.h"

struct DistanceReadings{
  float front;
  float back;
  float left;
  float right;
  Position position;

  DistanceReadings():
    front(0),
    back(0),
    left(0),
    right(0),
    position(0, 0, 0)
  {}

  DistanceReadings(float front, float back, float left, float right, Position position):
    front(front),
    back(back),
    left(left),
    right(right),
    position(position)
  {}
};
