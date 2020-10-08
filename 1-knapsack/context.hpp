#pragma once

#include <atomic>
#include <deque>
#include <queue>
#include <vector>

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

struct State {
 public:
  auto ComputeBound(const knapsack::Knapsack& ks) -> double;

 public:
  // current item in knapsack
  std::size_t cursor{0};

  // accumulated price and weight for current set of included items
  int current_price{0};
  int current_weight{0};

  // possible bound
  double bound{0};
};

auto operator<(const State& left, const State& right) -> bool;

using States = std::priority_queue<State>;

auto Split(States states, std::size_t batch_size) -> std::vector<States>;

struct Context {
  // executor / thread pool
  await::executors::IThreadPoolPtr tp;

  // knapsack and set of states
  const knapsack::Knapsack& knapsack;
  States states{};
};
