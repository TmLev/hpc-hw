#pragma once

#include <iostream>
#include <vector>

#include "config.hpp"

////////////////////////////////////////////////////////////////////////////////

struct Point {
 public:
  auto DistanceTo(const Point& other) const -> double;

  auto operator+=(const Point& other) -> Point&;

 public:
  alignas(config::hardware_destructive_interference_size) double x{0};
  alignas(config::hardware_destructive_interference_size) double y{0};
  config::Index cluster_index{0};
};

auto operator>>(std::istream& in, Point& p) -> std::istream&;
auto operator<<(std::ostream& out, const Point& p) -> std::ostream&;

using Points = std::vector<Point>;

////////////////////////////////////////////////////////////////////////////////

class Cluster {
 public:
  auto Centroid() -> Point&;
  auto Centroid() const -> const Point&;

  auto DistanceTo(const Point& p) const -> double;

  auto Add(const Point& p) -> void;
  auto Update() -> bool;

 private:
  Point centroid_{};
  Point update_{};
  alignas(config::hardware_destructive_interference_size) config::Size size_{0};
};

using Clusters = std::vector<Cluster>;
