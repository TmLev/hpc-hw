#include <chrono>
#include <cstdlib>
#include <iostream>

#include "config.hpp"
#include "solver.hpp"

// NOLINTNEXTLINE
using namespace config;

auto RunTests(std::ostream& log = std::cerr) -> Durations {
  auto durations = Durations{};

  for (auto test = 1; test <= kTestCount; ++test) {
    auto solver = Solver{};
    auto input = "data/" + std::to_string(test);
    auto output = "results/" + std::to_string(test);

    auto start = Clock::now();
    solver.Solve(input, output);
    auto dur = std::chrono::duration_cast<Mcs>(Clock::now() - start).count();
    durations.push_back(dur);

    log << "Test " << test << " finished" << '\n';
  }

  log << std::flush;

  return durations;
}

auto main() -> int {
  std::srand(11);

  auto durations = RunTests();

  for (auto dur : durations) {
    std::cout << dur << ", ";
  }
  std::cout << std::endl;
}
