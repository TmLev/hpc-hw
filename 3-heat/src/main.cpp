#include <iostream>
#include <vector>

#include <mpi.h>

////////////////////////////////////////////////////////////////////////////////

constexpr auto kPieces = std::size_t{11};
constexpr auto kLength = double{1};
constexpr auto kThermalDiffusivity = double{1};  // k
constexpr auto kSpaceStep = double{0.02};        // h
constexpr auto kTimeStep = double{0.0002};       // dt

////////////////////////////////////////////////////////////////////////////////

#define EXPECT_OK(call)                                             \
  do {                                                              \
    const auto ec = call;                                           \
    if (ec != 0) {                                                  \
      MPI_Abort(MPI_COMM_WORLD, ec);                                \
      const auto ec_str = std::to_string(ec);                       \
      throw std::runtime_error("MPI call failed: " #call + ec_str); \
    }                                                               \
  } while (false)

// Log variable to any std::ostream as `name = value`.
// Example:
// ```cpp
// auto x = 2;
// auto ss = std::stringstream{};
// ss << LOG(x);
// assert(ss.str() == "x = 2");
// ```
#define LOG(variable) #variable " = " << variable

////////////////////////////////////////////////////////////////////////////////

struct WorldGuard {
  WorldGuard(int c, char** v) : argc(c), argv(v) {
    std::cout << "Starting...\n";
    EXPECT_OK(MPI_Init(&argc, &argv));
  }

  ~WorldGuard() {
    MPI_Finalize();
    std::cout << "Finished\n";
  }

  int argc;
  char** argv;
};

////////////////////////////////////////////////////////////////////////////////

auto GetRank() -> std::size_t {
  auto rank = 0;
  EXPECT_OK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
  assert(rank >= 0);
  return static_cast<std::size_t>(rank);
}

auto GetProcessCount() -> std::size_t {
  auto process_count = 0;
  EXPECT_OK(MPI_Comm_size(MPI_COMM_WORLD, &process_count));
  assert(process_count > 0);
  return static_cast<std::size_t>(process_count);
}

////////////////////////////////////////////////////////////////////////////////

auto main(int argc, char* argv[]) -> int {
  auto guard = WorldGuard{argc, argv};

  auto rank = GetRank();
  auto process_count = GetProcessCount();
  auto rod = std::vector<double>(kPieces + 2, 0);
}
