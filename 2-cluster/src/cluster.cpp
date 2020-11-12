#include <cmath>
#include "cluster.hpp"

////////////////////////////////////////////////////////////////////////////////

auto Point::DistanceTo(const Point& other) const -> double {
  return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
}

auto Point::operator+=(const Point& other) -> Point& {
#pragma omp atomic
  x += other.x;

#pragma omp atomic
  y += other.y;

  return *this;
}

auto operator>>(std::istream& in, Point& p) -> std::istream& {
  in >> p.x >> p.y;
  return in;
}

auto operator<<(std::ostream& out, const Point& p) -> std::ostream& {
  out << p.x << " " << p.y;
  return out;
}

////////////////////////////////////////////////////////////////////////////////

auto Cluster::Centroid() -> Point& {
  return centroid_;
}

auto Cluster::Centroid() const -> const Point& {
  return centroid_;
}

auto Cluster::DistanceTo(const Point& p) const -> double {
  return centroid_.DistanceTo(p);
}

auto Cluster::Add(const Point& p) -> void {
  update_ += p;

#pragma omp atomic
  size_ += 1;
}

auto Cluster::Update() -> bool {
  update_.x /= static_cast<double>(size_);
  update_.y /= static_cast<double>(size_);

  auto dist = centroid_.DistanceTo(update_);

  centroid_ = std::exchange(update_, {});
  size_ = 0;

  return dist > 1e-6;
}
