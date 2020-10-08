#include "context.hpp"

auto MaxPrice::Get() -> int {
  return price_.load();
}

auto MaxPrice::Update(int price) -> void {
  auto old = price_.load();
  while (old < price && !price_.compare_exchange_strong(old, price)) {
  }
}

auto MaxPrice::Clear() -> void {
  price_.store(0);
}

auto Split(States states, std::size_t batch_size) -> std::vector<States> {
  if (states.empty()) {
    return {};
  }

  auto count = std::max(1ul, states.size() / batch_size);
  auto splits = std::vector<States>{count};

  for (auto i = std::size_t{0};; ++i) {
    if (i + 1 == count) {
      splits[i] = std::move(states);
      break;
    }

    for (auto j = std::size_t{0}; j < batch_size; ++j) {
      auto s = states.top();
      states.pop();
      splits[i].push(s);
    }
  }

  return splits;
}

auto State::ComputeBound(const knapsack::Knapsack& ks) -> double {
  if (current_weight > ks.capacity) {
    return 0;
  }

  auto cs = cursor;
  auto weight = current_weight;

  bound = static_cast<double>(current_price);

  while (++cs < ks.items.size() &&
         weight + ks.items[cs].weight <= ks.capacity) {
    weight += ks.items[cs].weight;
    bound += ks.items[cs].price;
  }

  if (cs < ks.items.size()) {
    bound += (ks.capacity - weight) * ks.items[cs].GetRank();
  }

  return bound;
}

auto operator<(const State& left, const State& right) -> bool {
  return left.bound < right.bound;
}
