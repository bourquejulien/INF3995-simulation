#pragma once

enum class Action {None, Identify, Start, Move, Stop, EmergencyStop, Return, ChooseAngle};

struct Command {
  std::string uri;
  Action action;
};
