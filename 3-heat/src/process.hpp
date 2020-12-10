#pragma once

#include <optional>
#include <vector>

#include <mpi.h>

#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////

class Process {
 public:
  using DVec = std::vector<Double>;
  using Requests = std::vector<MPI_Request>;

 public:
  Process();

  auto ComputeHeat() -> std::optional<DVec>;

 private:
  auto Step() -> void;
  auto Collect() -> std::optional<DVec>;

  // Calculations
  auto RecalculateMiddle() -> void;
  auto RecalculateEnds() -> void;
  auto Update() -> void;
  auto ZeroEnds() -> void;

  // MPI
  auto InitRequests() -> void;
  auto InitShareFirst() -> void;
  auto InitShareLast() -> void;
  auto StartRequests() -> void;
  auto WaitRequests() -> void;

  // Meta
  auto IsFirst() const -> bool;
  auto IsLast() const -> bool;
  auto ComputePiecesFor(std::size_t rank) const -> std::size_t;

 private:
  std::size_t rank_{0};
  std::size_t process_count_{0};
  std::size_t pieces_{0};
  DVec heat_;
  DVec next_heat_;
  Requests requests_;
};
