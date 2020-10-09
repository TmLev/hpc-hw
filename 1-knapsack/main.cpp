#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "solver.hpp"

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

auto RunTests(const std::string& type, std::ostream& log = std::cerr) -> void {
  auto test_count = (type == "small" ? 41 : 10);

  for (auto test = 1; test <= test_count; ++test) {
    auto solver = Solver{4, 512};
    auto got = solver.Solve(BuildTestFilename(type, test, "in"));

    log << "\r[" << type[0] << "] " << test;
    if (got == Expected(type, test)) {
      log << " / " << test_count;
    } else {
      log << " failed";
      break;
    }
  }

  log << '\n';
}

auto main() -> int {
  RunTests("small");
  RunTests("medium");
}
