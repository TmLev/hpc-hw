#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include <await/executors/static_thread_pool.hpp>

#include "context.hpp"
#include "knapsack.hpp"

////////////////////////////////////////////////////////////////////////////////
////
//// Config
////

static constexpr auto kBatchSize = 256;
static constexpr auto kThreadCount = 4;

MaxPrice max_price;

////////////////////////////////////////////////////////////////////////////////
////
//// Algorithms
////

auto SingleBranch(const knapsack::Knapsack& ks, State state, States* states)
    -> void {
  auto [price, weight] = ks.items[state.cursor];
  if (state.current_weight + weight <= ks.capacity) {
    max_price.Update(state.current_price + price);
  }

  // branch without item under cursor
  if (state.ComputeBound(ks) > max_price.Get()) {
    states->push(state);
  }

  // branch including item under cursor
  state.current_price += price;
  state.current_weight += weight;
  if (state.ComputeBound(ks) > max_price.Get()) {
    states->push(state);
  }
}

auto Branch(Context context, bool root = false) -> void;

auto BatchPost(Context context) -> void {
  for (const auto& s : Split(std::move(context.states), kBatchSize)) {
    auto ctx = context;
    ctx.states = s;
    context.tp->Execute([ctx = std::move(ctx)] { Branch(ctx); });
  }
}

auto Branch(Context context, bool root) -> void {
  auto states = std::move(context.states);
  if (root) {
    states.push({});
  }

  while (!states.empty()) {
    auto state = states.top();
    states.pop();

    root ? root = false : state.cursor += 1;
    SingleBranch(context.knapsack, state, &states);

    if (states.size() >= kThreadCount * kBatchSize) {
      break;
    }
  }

  context.states = std::move(states);
  BatchPost(std::move(context));
}

auto Solve(const std::string& filename) -> void {
  auto knapsack = knapsack::ReadFrom(filename);
  if (knapsack.TooHeavyItems()) {
    return;
  }
  if (knapsack.AllItemsFit()) {
    max_price.Update(knapsack.GetTotalPrice());
    return;
  }

  knapsack.SortItems();

  auto tp = await::executors::MakeStaticThreadPool(kThreadCount);
  auto context = Context{tp, knapsack};
  tp->Execute([context] { Branch(context, /*root=*/true); });
  tp->Join();
}

////////////////////////////////////////////////////////////////////////////////
////
//// Tests
////

auto GetAnswer(const std::string& type, int test) -> int {
  auto f = std::fstream{"tests/" + type + "/" + std::to_string(test) + ".out"};
  auto answer = 0;
  f >> answer;
  return answer;
}

auto RunTests(const std::string& type) -> void {
  std::cerr << "[" << type[0] << "] ";

  auto last = (type == "small" ? 41 : 10);
  for (auto test = 1; test <= last; ++test) {
    max_price.Clear();
    Solve("tests/" + type + "/" + std::to_string(test) + ".in");
    std::cerr << (max_price.Get() == GetAnswer(type, test) ? "." : "F");
  }

  std::cerr << '\n';
}

////////////////////////////////////////////////////////////////////////////////
////
//// main
////

auto main() -> int {
  RunTests("small");
  RunTests("medium");
}
