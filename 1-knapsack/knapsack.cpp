#include <algorithm>
#include <fstream>
#include <numeric>

#include "knapsack.hpp"

////////////////////////////////////////////////////////////////////////////////

auto Item::GetRank() const -> double {
  return static_cast<double>(price) / weight;
}

auto operator>(const Item& left, const Item& right) -> bool {
  return left.GetRank() > right.GetRank();
}

auto operator>>(std::istream& in, Item& item) -> std::istream& {
  in >> item.price >> item.weight;
  return in;
}

////////////////////////////////////////////////////////////////////////////////

auto Knapsack::TooHeavyItems() const -> bool {
  return std::all_of(std::begin(items), std::end(items),
                     [&](const Item& item) { return item.weight > capacity; });
}

auto Knapsack::AllItemsFit() const -> bool {
  const auto total_weight = std::accumulate(std::begin(items), std::end(items),
                                            0, [](int accum, const Item& item) {
                                              accum += item.weight;
                                              return accum;
                                            });
  return total_weight <= capacity;
}

auto Knapsack::GetTotalPrice() const -> int {
  return std::accumulate(std::begin(items), std::end(items), 0,
                         [](int accum, const Item& item) {
                           accum += item.price;
                           return accum;
                         });
}

auto Knapsack::SortItems() -> void {
  std::sort(std::begin(items), std::end(items), std::greater{});
}

auto operator>>(std::istream& in, Knapsack& ks) -> std::istream& {
  auto count = std::size_t{0};
  in >> count >> ks.capacity;

  ks.items = Items{count};
  for (auto& i : ks.items) {
    in >> i;
  }

  return in;
}

auto ReadFrom(const std::string& filename) -> Knapsack {
  auto ks = Knapsack{};

  auto input = std::fstream{filename};
  input >> ks;

  return ks;
}
