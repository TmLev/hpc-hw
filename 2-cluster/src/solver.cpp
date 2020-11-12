#include <cstdlib>
#include <fstream>
#include <unordered_set>

#include "solver.hpp"

////////////////////////////////////////////////////////////////////////////////

Solver::Solver(std::size_t thread_count) : thread_count_(thread_count) {
}

auto Solver::Solve(const std::string& input, const std::string& output)
    -> void {
  Read(input);

  ChooseRandomCentroids();

  auto updated = true;
  while (updated) {
    updated = Step();
  }

  Write(output);
}

////////////////////////////////////////////////////////////////////////////////

auto Solver::Read(const std::string& input) -> void {
  auto in = std::ifstream{input};

  auto point_count = config::Size{0};
  auto cluster_count = config::Size{0};

  in >> point_count >> cluster_count;

  assert(point_count > 0);
  assert(cluster_count > 0);

  points_.resize(point_count);
  for (auto& p : points_) {
    in >> p;
  }

  clusters_.resize(cluster_count);
}

// Use of `rand()` here is intentional.
// Serious randomness (via `mt19937`, for example) is not required.
auto Solver::ChooseRandomCentroids() -> void {
  auto used = std::unordered_set<config::Index>{};

  while (used.size() < clusters_.size()) {
    auto p = static_cast<config::Index>(std::rand()) % points_.size();
    if (used.find(p) != std::end(used)) {
      continue;
    }

    clusters_[used.size()].Centroid() = points_[p];
    used.insert(p);
  }
}

auto Solver::Step() -> bool {
  for (auto p = config::Index{0}; p < points_.size(); ++p) {
    Assign(points_[p]);
  }

  auto updated = false;
  for (auto c = config::Index{0}; c < clusters_.size(); ++c) {
    updated |= clusters_[c].Update();
  }

  return updated;
}

auto Solver::Assign(Point& p) -> void {
  auto nearest = config::Index{0};
  auto min_dist = clusters_[nearest].DistanceTo(p);

  for (auto i = config::Index{0}; i < clusters_.size(); ++i) {
    if (auto dist = clusters_[i].DistanceTo(p); dist < min_dist) {
      nearest = i;
      min_dist = dist;
    }
  }

  p.cluster_index = nearest;
  clusters_[nearest].Add(p);
}

auto Solver::Write(const std::string& output) -> void {
  auto out = std::ofstream{output};

  for (const auto& c : clusters_) {
    out << c.Centroid() << '\n';
  }

  for (const auto& p : points_) {
    out << p.cluster_index << '\n';
  }
}
