#include <iostream>
#include <sstream>

#include <mpi.h>

#include "world-guard.hpp"
#include "process.hpp"

////////////////////////////////////////////////////////////////////////////////

auto ToString(const Process::DVec& heat, std::string_view delim = " ")
    -> std::string {
  auto ss = std::stringstream{};
  auto first = true;

  for (const auto& h : heat) {
    if (!first) {
      ss << delim;
    } else {
      first = false;
    }

    ss << h;
  }

  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

auto main(int argc, char** argv) -> int {
  const auto guard = WorldGuard{argc, argv};

  auto process = Process();

  const auto begin = MPI_Wtime();
  const auto heat = process.ComputeHeat();
  const auto dur = MPI_Wtime() - begin;

  if (heat) {
    std::cout << "Finished in " << dur << " seconds" << '\n';
    std::cout << ToString(*heat) << '\n';
  }
}
