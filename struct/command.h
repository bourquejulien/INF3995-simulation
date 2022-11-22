#pragma once

enum class Action {None, Identify, Start, Move, Stop, EmergencyStop, ChooseAngle};

struct Command {
  std::string uri;
  Action action;
};
