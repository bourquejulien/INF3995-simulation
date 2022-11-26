#pragma once

enum class Action {None, Identify, Start, Move, Stop, EmergencyStop, Return, ChooseAngle, Done};

struct Command {
  std::string uri;
  Action action;
};
