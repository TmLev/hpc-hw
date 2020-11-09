#include <iostream>
#include <omp.h>

auto main() -> int {
  std::cout << "Procs: " << omp_get_num_procs() << '\n';
}
