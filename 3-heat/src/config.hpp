#pragma once

#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace config {

constexpr auto kLeftEnd = Double{0};
constexpr auto kRightEnd = Double{1};
constexpr auto kLength = kRightEnd - kLeftEnd;
constexpr auto kPieces = std::size_t{50 + 1};
constexpr auto kSpaceStep = kLength / (kPieces - 1);  // h

constexpr auto kEnvHeat = Double{0};             // environment temperature
constexpr auto kInitHeat = Double{1};            // u_0
constexpr auto kThermalDiffusivity = Double{1};  // k

constexpr auto kTimeStep =
    kSpaceStep * kSpaceStep / kThermalDiffusivity / 2;       // dt
constexpr auto kTimeBegin = Double{0};                       // t_0
constexpr auto kTimeEnd = Double{static_cast<Double>(0.1)};  // T

}  // namespace config
