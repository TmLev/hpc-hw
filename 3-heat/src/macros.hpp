#pragma once

#include <string>

#include <mpi.h>

////////////////////////////////////////////////////////////////////////////////

#define EXPECT_OK(call)                                             \
  do {                                                              \
    const auto ec = call;                                           \
    if (ec != MPI_SUCCESS) {                                        \
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
