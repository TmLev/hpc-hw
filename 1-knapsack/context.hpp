#pragma once

#include <atomic>

#include <await/executors/thread_pool.hpp>

#include "knapsack.hpp"

struct MaxPrice {
 public:
  auto Get() -> int;
  auto Update(int price) -> void;
  auto Clear() -> void;

 private:
  std::atomic<int> price_{0};
};

struct Context {
  // executor / thread pool
  await::executors::IThreadPoolPtr tp;

  // knapsack and current item
  const knapsack::Knapsack& knapsack;
  std::size_t cursor{0};

  // accumulated price and weight for current set of included items
  int current_price{0};
  int current_weight{0};
};
