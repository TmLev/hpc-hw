#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>

#include <await/executors/static_thread_pool.hpp>

#include "context.hpp"
#include "knapsack.hpp"

std::mutex mutex;
int max_price{0};

auto GetMaxPrice() -> int {
  auto lock = std::unique_lock{mutex};
  return max_price;
}

auto UpdateMaxPrice(int price) -> void {
  auto lock = std::unique_lock{mutex};
  max_price = std::max(price, max_price);
}

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
    UpdateMaxPrice(context.current_price + price);
  }

  // branch without current node
  if (Bound(context) > GetMaxPrice()) {
    context.tp->Execute([context] { Branch(context); });
  }

  // branch including current node
  context.current_price += price;
  context.current_weight += weight;
  if (Bound(context) > GetMaxPrice()) {
    context.tp->Execute([context] { Branch(context); });
  }
}

auto Solve(const std::string& filename) -> void {
  auto knapsack = knapsack::ReadFrom(filename);
  if (knapsack.TooHeavyItems()) {
    return;
  }
  if (knapsack.AllItemsFit()) {
    max_price = knapsack.GetTotalPrice();
    return;
  }

  knapsack.SortItems();

  auto tp = await::executors::MakeStaticThreadPool(4, "default");
  auto context = Context{tp, knapsack};
  tp->Execute([context] { Branch(context, /*root=*/ true); });
  tp->Join();
}

auto Answer(int test) -> int {
  auto f = std::fstream{"tests/small/" + std::to_string(test) + ".out"};
  auto answer = 0;
  f >> answer;
  return answer;
}

auto main() -> int {
  for (auto test = 1; test <= 41; ++test) {
    Solve("tests/small/" + std::to_string(test) + ".in");
    std::cerr << (max_price == Answer(test) ? "." : "F") << std::flush;
    max_price = 0;
  }
}
