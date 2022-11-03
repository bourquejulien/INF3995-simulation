#pragma once

enum class Action {None, Identify, Start, Stop, ChooseAngle, Move};

struct Command {
  std::string uri;
  Action action;
};
