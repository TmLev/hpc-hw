#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace config {

constexpr auto kLeftEnd = double{0};
constexpr auto kRightEnd = double{1};
constexpr auto kLength = kRightEnd - kLeftEnd;
constexpr auto kPieces = std::size_t{51};
constexpr auto kSpaceStep = kLength / (kPieces - 1);  // h

constexpr auto kEnvHeat = double{0};             // environment temperature
constexpr auto kInitHeat = double{1};            // u_0
constexpr auto kThermalDiffusivity = double{1};  // k

constexpr auto kTimeStep = double{0.0002};  // dt
constexpr auto kTimeBegin = double{0};      // t_0
constexpr auto kTimeEnd = double{0.1};      // T

}  // namespace config
