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
