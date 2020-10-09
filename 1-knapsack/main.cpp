#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "solver.hpp"

using Clock = std::chrono::steady_clock;
using Mcs = std::chrono::microseconds;
using Durations = std::vector<std::result_of_t<decltype (&Mcs::count)(Mcs)>>;

auto BuildTestFilename(const std::string& type, int test,
                       const std::string& extension) -> std::string {
  return "tests/" + type + "/" + std::to_string(test) + "." + extension;
}

auto Expected(const std::string& type, int test) -> int {
  auto f = std::fstream{BuildTestFilename(type, test, "out")};
  if (auto answer = 0; f >> answer) {
    return answer;
  } else {
    throw std::runtime_error("Failed to read answer from .out");
  }
}

auto RunTests(const std::string& type, std::ostream& log = std::cerr)
    -> Durations {
  auto durations = Durations{};

  auto test_count = (type == "small" ? 41 : 10);
  for (auto test = 1; test <= test_count; ++test) {
    auto solver = Solver{4, 1024};

    auto start = Clock::now();
    auto got = solver.Solve(BuildTestFilename(type, test, "in"));
    auto dur = std::chrono::duration_cast<Mcs>(Clock::now() - start).count();
    durations.push_back(dur);

    log << "\r[" << type[0] << "] " << test;
    if (got == Expected(type, test)) {
      log << " / " << test_count;
    } else {
      log << " failed";
      break;
    }
  }

  log << std::endl;

  return durations;
}

auto main() -> int {
  auto benchmarks = std::unordered_map<std::string, Durations>{};
  auto test_types = {"small", "medium"};

  for (auto type : test_types) {
    benchmarks[type] = RunTests(type);
  }

  for (auto type : test_types) {
    std::cout << "[" << type[0] << "] ";
    for (auto m : benchmarks[type]) {
      std::cout << m << ", ";
    }
    std::cout << std::endl;
  }
}
