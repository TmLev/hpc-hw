#include <fstream>

#include "worker.hpp"

Worker::Worker(Rank rank) : rank_(rank) {
}

auto Worker::MasterInit(const std::string& input) -> void {
  assert(rank_ == 0);

  auto in = std::fstream{input};
  auto settings = board::Settings{};

  in >> settings.rows >> settings.columns;
  assert(settings.rows > 0);
  assert(settings.columns > 0);

  auto board = board::Board{settings};
  in >> board;
}
