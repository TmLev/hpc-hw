#include <chrono>
#include <cstdlib>
#include <iostream>

#include "config.hpp"
#include "solver.hpp"

// NOLINTNEXTLINE
using namespace config;

auto RunTests(std::ostream& log = std::cerr) -> void {
  for (auto test = 1; test <= kTestCount; ++test) {
    auto solver = Solver{};
    auto input = "data/" + std::to_string(test);
    auto output = "results/" + std::to_string(test);

    auto start = Clock::now();
    solver.Solve(input, output);
    auto dur = std::chrono::duration_cast<Mcs>(Clock::now() - start).count();

    log << dur << (test == kTestCount ? "" : ", ");
  }

  log << std::flush;
}

auto main() -> int {
  std::srand(11);
  RunTests();
}
