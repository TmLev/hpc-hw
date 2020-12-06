#pragma once

#include <iostream>

////////////////////////////////////////////////////////////////////////////////

struct WorldGuard {
  WorldGuard(int c, char** v, std::ostream& l = std::cerr);
  ~WorldGuard();

  int argc;
  char** argv;
  std::ostream& log;
};
