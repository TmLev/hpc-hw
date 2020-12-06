#include <algorithm>
#include <cmath>

#include <mpi.h>

#include "config.hpp"
#include "macros.hpp"
#include "process.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace config;  // NOLINT

////////////////////////////////////////////////////////////////////////////////

auto GetRank() -> std::size_t {
  auto rank = -1;
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

auto IsFirstProcess(std::size_t rank) -> bool {
  return rank == 0;
}

auto IsLastProcess(std::size_t rank, std::size_t process_count) -> bool {
  return rank + 1 == process_count;
}

////////////////////////////////////////////////////////////////////////////////

Process::Process() {
  rank_ = GetRank();
  process_count_ = GetProcessCount();
  pieces_ = ComputePiecesFor(rank_);

  // `heat` represents pieces of rod. `.front()` and `.back()` are used for
  // message passing between neighbours, hence + 2.
  heat_ = DVec(pieces_ + 2, kInitHeat);
  next_heat_ = DVec(pieces_ + 2, 0);

  ZeroEnds();
}

////////////////////////////////////////////////////////////////////////////////

auto Process::ComputeHeat() -> std::optional<Process::DVec> {
  for (auto time = kTimeBegin; time < kTimeEnd; time += kTimeStep) {
    Step();
  }

  return Collect();
}

////////////////////////////////////////////////////////////////////////////////

auto Process::Step() -> void {
  SendRecvLast();
  SendRecvFirst();
  Recalculate();
  Update();
}

// Collect data from all processes. First process is master.
auto Process::Collect() -> std::optional<DVec> {
  auto result = DVec{};
  auto recvcounts = std::vector<int>{static_cast<int>(pieces_)};
  auto displs = std::vector<int>{0};

  if (IsFirst()) {
    result = DVec(kPieces, 0);
    recvcounts.reserve(process_count_);
    displs.reserve(process_count_);

    for (auto source = 1; source < process_count_; ++source) {        // NOLINT
      const auto count = static_cast<int>(ComputePiecesFor(source));  // NOLINT
      const auto displ = displs.back() + recvcounts.back();
      recvcounts.push_back(count);
      displs.push_back(displ);
    }
  }

  // NOLINTNEXTLINE
  EXPECT_OK(MPI_Gatherv(&heat_[1], pieces_, MPI_DOUBLE, result.data(),
                        recvcounts.data(), displs.data(), MPI_DOUBLE, 0,
                        MPI_COMM_WORLD));

  return result;
}

////////////////////////////////////////////////////////////////////////////////

// Send `heat[last]` to neighbour `rank + 1`.
// Receive message from neighbour `rank - 1` to `heat.front()`.
auto Process::SendRecvLast() -> void {
  const auto tag = 1;

  // Send – all except last.
  if (!IsLast()) {
    const auto dest = static_cast<int>(rank_) + 1;
    EXPECT_OK(
        MPI_Send(&heat_[pieces_], 1, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD));
  }

  // Receive – all except first.
  if (!IsFirst()) {
    const auto source = static_cast<int>(rank_) - 1;
    EXPECT_OK(MPI_Recv(&heat_.front(), 1, MPI_DOUBLE, source, tag,
                       MPI_COMM_WORLD, nullptr));
  }
}

// Send `heat[first]` to neighbour `rank - 1`.
// Receive message from neighbour `rank + 1` to `heat.back()`.
auto Process::SendRecvFirst() -> void {
  const auto tag = 2;

  // Send – all except first.
  if (!IsFirst()) {
    const auto dest = static_cast<int>(rank_) - 1;
    EXPECT_OK(MPI_Send(&heat_[1], 1, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD));
  }

  // Receive – all except last.
  if (!IsLast()) {
    const auto source = static_cast<int>(rank_) + 1;
    EXPECT_OK(MPI_Recv(&heat_.back(), 1, MPI_DOUBLE, source, tag,
                       MPI_COMM_WORLD, nullptr));
  }
}

// Recalculate heat for interval `[1, process_pieces]`.
auto Process::Recalculate() -> void {
  for (auto i = std::size_t{1}; i <= pieces_; ++i) {
    const auto n = heat_[i] + kThermalDiffusivity * kTimeStep /
                                  std::pow(kSpaceStep, 2) *
                                  (heat_[i + 1] - 2 * heat_[i] + heat_[i - 1]);
    next_heat_[i] = std::clamp(n, kEnvHeat, kInitHeat);
  }
}

auto Process::Update() -> void {
  std::copy(std::begin(next_heat_), std::end(next_heat_), std::begin(heat_));
  ZeroEnds();
}

auto Process::ZeroEnds() -> void {
  if (IsFirst()) {
    heat_[0] = 0;
    heat_[1] = 0;
  }

  if (IsLast()) {
    heat_[pieces_] = 0;
    heat_[pieces_ + 1] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

auto Process::IsFirst() const -> bool {
  return IsFirstProcess(rank_);
}

auto Process::IsLast() const -> bool {
  return IsLastProcess(rank_, process_count_);
}

auto Process::ComputePiecesFor(std::size_t rank) const -> std::size_t {
  const auto quotient = kPieces / process_count_;

  if (IsLastProcess(rank, process_count_)) {
    return quotient + kPieces % process_count_;
  } else {
    return quotient;
  }
}
