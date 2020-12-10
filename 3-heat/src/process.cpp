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

auto HeatFormula(Double left, Double middle, Double right) -> Double {
  return middle + kThermalDiffusivity * kTimeStep / std::pow(kSpaceStep, 2) *
                      (right - 2 * middle + left);
}

////////////////////////////////////////////////////////////////////////////////

Process::Process() {
  rank_ = GetRank();
  process_count_ = GetProcessCount();
  pieces_ = ComputePiecesFor(rank_);

  // `heat` represents pieces of rod. `.front()` and `.back()` are used for
  // message passing between neighbours, hence `+ 2`.
  heat_ = DVec(pieces_ + 2, kInitHeat);
  next_heat_ = DVec(pieces_ + 2, 0);
  ZeroEnds();

  // `requests` save MPI Send/Recv requests to do them in asynchronous fashion.
  InitRequests();
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
  StartRequests();
  RecalculateMiddle();
  WaitRequests();
  RecalculateEnds();
  Update();
}

// Collect data from all processes. First process is master.
auto Process::Collect() -> std::optional<DVec> {
  auto result = std::optional<DVec>{};
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
  EXPECT_OK(MPI_Gatherv(&heat_[1], pieces_, DOUBLE,
                        result ? result->data() : nullptr, recvcounts.data(),
                        displs.data(), DOUBLE, 0, MPI_COMM_WORLD));

  return result;
}

////////////////////////////////////////////////////////////////////////////////

// Recalculate heat for interval `[2, process_pieces - 1]`.
auto Process::RecalculateMiddle() -> void {
  for (auto i = std::size_t{2}; i + 1 <= pieces_; ++i) {
    const auto h = HeatFormula(heat_[i - 1], heat_[i], heat_[i + 1]);
    next_heat_[i] = std::clamp(h, kEnvHeat, kInitHeat);
  }
}

// Recalculate heat for the leftmost and the rightmost points.
auto Process::RecalculateEnds() -> void {
  for (auto i : {std::size_t{1}, pieces_}) {
    const auto h = HeatFormula(heat_[i - 1], heat_[i], heat_[i + 1]);
    next_heat_[i] = std::clamp(h, kEnvHeat, kInitHeat);
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

auto Process::InitRequests() -> void {
  InitShareFirst();
  InitShareLast();
}

// Send `heat[first]` to neighbour `rank - 1`.
// Receive message from neighbour `rank + 1` to `heat.back()`.
auto Process::InitShareFirst() -> void {
  const auto tag = 2;

  // Send – all except first.
  if (!IsFirst()) {
    const auto dest = static_cast<int>(rank_) - 1;
    auto request = MPI_Request{};
    EXPECT_OK(MPI_Send_init(&heat_[1], 1, DOUBLE, dest, tag, MPI_COMM_WORLD,
                            &request));
    requests_.push_back(std::move(request));
  }

  // Receive – all except last.
  if (!IsLast()) {
    const auto source = static_cast<int>(rank_) + 1;
    auto request = MPI_Request{};
    EXPECT_OK(MPI_Recv_init(&heat_.back(), 1, DOUBLE, source, tag,
                            MPI_COMM_WORLD, &request));
    requests_.push_back(std::move(request));
  }
}

// Send `heat[last]` to neighbour `rank + 1`.
// Receive message from neighbour `rank - 1` to `heat.front()`.
auto Process::InitShareLast() -> void {
  const auto tag = 1;

  // Send – all except last.
  if (!IsLast()) {
    const auto dest = static_cast<int>(rank_) + 1;
    auto request = MPI_Request{};
    EXPECT_OK(MPI_Send_init(&heat_[pieces_], 1, DOUBLE, dest, tag,
                            MPI_COMM_WORLD, &request));
    requests_.push_back(std::move(request));
  }

  // Receive – all except first.
  if (!IsFirst()) {
    const auto source = static_cast<int>(rank_) - 1;
    auto request = MPI_Request{};
    EXPECT_OK(MPI_Recv_init(&heat_.front(), 1, DOUBLE, source, tag,
                            MPI_COMM_WORLD, &request));
    requests_.push_back(std::move(request));
  }
}

auto Process::StartRequests() -> void {
  if (requests_.empty()) {
    return;
  }

  const auto count = static_cast<int>(requests_.size());
  EXPECT_OK(MPI_Startall(count, requests_.data()));
}

auto Process::WaitRequests() -> void {
  if (requests_.empty()) {
    return;
  }

  const auto count = static_cast<int>(requests_.size());
  EXPECT_OK(MPI_Waitall(count, requests_.data(), MPI_STATUSES_IGNORE));
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
