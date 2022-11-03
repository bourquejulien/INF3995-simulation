#ifndef DISTANCEREADINGS_H
#define DISTANCEREADINGS_H

struct DistanceReadings{
  float front;
  float back;
  float left;
  float right;

  DistanceReadings():
    front(0),
    back(0),
    left(0),
    right(0)
  {}

  DistanceReadings(float front, float back, float left, float right):
    front(front),
    back(back),
    left(left),
    right(right)
  {}
};

#endif