#pragma once

#include <optional>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

class Process {
 public:
  using DVec = std::vector<double>;

 public:
  Process();

  auto ComputeHeat() -> std::optional<DVec>;

 private:
  auto Step() -> void;
  auto Collect() -> std::optional<DVec>;

  // Step impl
  auto SendRecvLast() -> void;
  auto SendRecvFirst() -> void;
  auto Recalculate() -> void;
  auto Update() -> void;
  auto ZeroEnds() -> void;

  // Collect impl
  auto SendAll(int tag) -> void;
  auto RecvAll(int tag) -> DVec;

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
};
