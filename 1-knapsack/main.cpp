#include <algorithm>
#include <fstream>
#include <iostream>

#include <await/executors/static_thread_pool.hpp>

#include "context.hpp"
#include "knapsack.hpp"

MaxPrice max_price;

auto Bound(Context context) -> double {
  if (context.current_weight > context.knapsack.capacity) {
    return 0;
  }

  const auto& ks = context.knapsack;
  auto& cs = context.cursor;

  auto current_price = static_cast<double>(context.current_price);
  auto current_weight = context.current_weight;

  while (++cs < ks.items.size() &&
         current_weight + ks.items[cs].weight <= ks.capacity) {
    current_price += ks.items[cs].price;
    current_weight += ks.items[cs].weight;
  }

  if (cs < ks.items.size()) {
    current_price += (ks.capacity - current_weight) * ks.items[cs].GetRank();
  }

  return current_price;
}

auto Branch(Context context, bool root = false) -> void {
  // no branching in the last node
  if (context.cursor + 1 >= context.knapsack.items.size()) {
    return;
  }

  if (!root) {
    context.cursor += 1;
  }

  auto [price, weight] = context.knapsack.items[context.cursor];
  if (context.current_weight + weight <= context.knapsack.capacity) {
    max_price.Update(context.current_price + price);
  }

  // branch without item under cursor
  if (Bound(context) > max_price.Get()) {
    context.tp->Execute([context] { Branch(context); });
  }

  // branch including item under cursor
  context.current_price += price;
  context.current_weight += weight;
  if (Bound(context) > max_price.Get()) {
    context.tp->Execute([context] { Branch(context); });
  }
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

  auto tp = await::executors::MakeStaticThreadPool(1);
  auto context = Context{tp, knapsack};
  tp->Execute([context] { Branch(context, /*root=*/ true); });
  tp->Join();
}

auto SmallAnswer(int test) -> int {
  auto f = std::fstream{"tests/small/" + std::to_string(test) + ".out"};
  auto answer = 0;
  f >> answer;
  return answer;
}

auto SmallTests() -> void {
  for (auto test = 1; test <= 41; ++test) {
    Solve("tests/small/" + std::to_string(test) + ".in");
    std::cerr << (max_price.Get() == SmallAnswer(test) ? "." : "F");
    max_price.Clear();
  }
}

auto MediumTests() -> void {
  for (auto test = 1; test <= 1; ++test) {
    Solve("tests/medium/test_100_" + std::to_string(test) + ".in");
  }
}

auto main() -> int {
  SmallTests();
}
