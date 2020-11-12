#pragma once

#include "worker.hpp"

////////////////////////////////////////////////////////////////////////////////

struct Env {
  int argc;
  char** argv;
};

////////////////////////////////////////////////////////////////////////////////

class World {
 public:
  using ErrorCode = int;
  using Size = int;

 public:
  explicit World(Env env);
  ~World();

  auto NextGeneration() -> void;

 private:
  auto Abort(ErrorCode ec) -> void;

 private:
  Size size_;
  Worker worker_;
};
