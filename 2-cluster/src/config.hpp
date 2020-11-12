#pragma once

#include <cstddef>
#include <new>

namespace config {

using Size = std::size_t;
using Index = std::size_t;

using Clock = std::chrono::steady_clock;
using Mcs = std::chrono::microseconds;
using Durations = std::vector<std::result_of_t<decltype (&Mcs::count)(Mcs)>>;

constexpr auto kTestCount = 5;
constexpr auto kThreadCount = 4;

#if __cpp_lib_hardware_interference_size > 201703L

using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;

#else

// Lucky guess │ __cacheline_aligned │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ ...

// NOLINTNEXTLINE
constexpr std::size_t hardware_constructive_interference_size =
    2 * sizeof(std::max_align_t);

// NOLINTNEXTLINE
constexpr std::size_t hardware_destructive_interference_size =
    2 * sizeof(std::max_align_t);

#endif

}  // namespace config
