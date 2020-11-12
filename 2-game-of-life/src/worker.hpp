#pragma once

#include <vector>

#include "board.hpp"

class Worker {
 public:
  using Rank = int;

 public:
  Worker(Rank rank = 0);

  auto MasterInit(const std::string& input) -> void;

 private:
  Rank rank_;
  board::Board slice_;
};
