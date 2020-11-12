#pragma once

#include "config.hpp"
#include "cluster.hpp"

class Solver {
 public:
  explicit Solver(config::Size thread_count);

  auto Solve(const std::string& input, const std::string& output) -> void;

 private:
  auto Read(const std::string& input) -> void;

  auto ChooseRandomCentroids() -> void;

  auto Step() -> bool;

  auto Assign(Point& p) -> void;

  auto Write(const std::string& output) -> void;

 private:
  const config::Size thread_count_;

  Points points_;
  Clusters clusters_;
};
