#ifndef POSITION_H
#define POSITION_H

struct Position{
  float posX;
  float posY;
  float posZ;

  Position(float x, float y, float z):
    posX(x),
    posY(y),
    posZ(z)
  {}
};

#endif
