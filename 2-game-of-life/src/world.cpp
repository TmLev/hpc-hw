#include <iostream>

#include <mpi.h>

#include "board.hpp"
#include "world.hpp"

////////////////////////////////////////////////////////////////////////////////

#define EXPECT_MPI_OK(call)                   \
  do {                                        \
    auto ec = call;                           \
    if (ec != 0) {                            \
      Abort(ec);                              \
      throw std::runtime_error("MPI failed"); \
    }                                         \
  } while (false)

////////////////////////////////////////////////////////////////////////////////

World::World(Env env) {
  EXPECT_MPI_OK(MPI_Init(&env.argc, &env.argv));
  EXPECT_MPI_OK(MPI_Comm_size(MPI_COMM_WORLD, &size_));

  auto rank = Worker::Rank{};
  EXPECT_MPI_OK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
  worker_ = Worker{rank};

  if (rank == 0) {
    const auto input = std::string{(*env.argv)[1]};
    worker_.MasterInit(input);
  }
}

World::~World() {
  MPI_Finalize();
}

auto World::NextGeneration() -> void {
}

////////////////////////////////////////////////////////////////////////////////

auto World::Abort(ErrorCode ec) -> void {
  MPI_Abort(MPI_COMM_WORLD, ec);
}
