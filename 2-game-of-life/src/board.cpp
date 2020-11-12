#include "board.hpp"

namespace board {

Board::Board(Settings settings)
    : settings_(settings), data_(settings.rows * settings.columns, 0) {
}

auto Board::Rows() const -> Size {
  return settings_.rows;
}

auto Board::Columns() const -> Size {
  return settings_.columns;
}

auto Board::RawData() -> Element* {
  return data_.data();
}

auto Board::operator()(Index i, Index j) -> Element& {
  return data_[i * settings_.rows + j];
}

auto operator>>(std::istream& in, Board& board) -> std::istream& {
  // TODO(TmLev): commit to some input/output format or use config

  for (auto row = Size{0}; row < board.Rows(); ++row) {
    for (auto column = Size{0}; column < board.Columns(); ++column) {
      auto c = char{};
      in >> c;
      board(row, column) = c - '0';
    }
  }

  return in;
}

}  // namespace board
