#include <iostream>
#include <thread>

#include <mpi.h>

#include "macros.hpp"
#include "world-guard.hpp"

////////////////////////////////////////////////////////////////////////////////

WorldGuard::WorldGuard(int c, char** v, std::ostream& l)
    : argc(c), argv(v), log(l) {
  log << "[T " << std::this_thread::get_id() << "] started\n";
  EXPECT_OK(MPI_Init(&argc, &argv));
}

WorldGuard::~WorldGuard() {
  MPI_Finalize();
  log << "[T " << std::this_thread::get_id() << "] finished\n";
}
