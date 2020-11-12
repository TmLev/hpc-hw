#pragma once

#include <iostream>
#include <vector>

namespace board {

using Size = std::size_t;
using Index = std::size_t;

using Element = int;
using Data = std::vector<Element>;

struct Settings {
  Size rows{0};
  Size columns{0};
};

class Board {
 public:
  explicit Board(Settings settings = {});

  auto Rows() const -> Size;
  auto Columns() const -> Size;

  auto RawData() -> Element*;

  auto operator()(Index i, Index j) -> Element&;

 private:
  Settings settings_;
  Data data_;
};

auto operator>>(std::istream& in, Board& board) -> std::istream&;

}  // namespace board
