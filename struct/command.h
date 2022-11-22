#pragma once

enum class Action {None, Identify, Start, Move, Stop, EmergencyStop, ChooseAngle, ChoosePerpendicularAngle};

struct Command {
  std::string uri;
  Action action;
};
