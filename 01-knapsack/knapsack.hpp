#pragma once

#include <string>
#include <vector>

namespace knapsack {

struct Item {
 public:
  auto GetRank() const -> double;

 public:
  int price;
  int weight;
};

using Items = std::vector<Item>;

auto operator>(const Item& left, const Item& right) -> bool;
auto operator>>(std::istream& in, Item& item) -> std::istream&;
auto operator<<(std::ostream& out, const Item& item) -> std::ostream&;

struct Knapsack {
 public:
  auto TooHeavyItems() const -> bool;
  auto AllItemsFit() const -> bool;

  auto GetTotalPrice() const -> int;

  auto SortItems() -> void;

 public:
  Items items;
  int capacity;
};

auto operator>>(std::istream& in, Knapsack& ks) -> std::istream&;

auto ReadFrom(const std::string& filename) -> Knapsack;

}  // namespace knapsack
