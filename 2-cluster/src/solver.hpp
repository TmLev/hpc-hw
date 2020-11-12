#pragma once

#include "config.hpp"
#include "cluster.hpp"

class Solver {
 public:
  auto Solve(const std::string& input, const std::string& output) -> void;

 private:
  auto Read(const std::string& input) -> void;

  auto ChooseRandomCentroids() -> void;

  auto Step() -> bool;

  auto Assign(Point& p) -> void;

  auto Write(const std::string& output) -> void;

 private:
  Points points_;
  Clusters clusters_;
};
