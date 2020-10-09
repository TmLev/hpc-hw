#pragma once

#include "context.hpp"
#include "knapsack.hpp"

#include <await/executors/thread_pool.hpp>

class Solver {
 public:
  Solver(std::size_t thread_count = 1, std::size_t batch_size = 512);

  // Solve is NOT thread-safe
  auto Solve(const std::string& filename) -> int;

 private:
  auto Branch(States states, bool root = false) -> void;
  auto SingleBranch(State state, States* states) -> void;
  auto BatchPost(States states) -> void;

 private:
  MaxPrice max_price_{};

  await::executors::IThreadPoolPtr tp_{nullptr};
  Knapsack knapsack_{};

  const std::size_t thread_count_;
  const std::size_t batch_size_;
};
