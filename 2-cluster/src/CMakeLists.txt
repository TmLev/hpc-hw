find_package(OpenMP REQUIRED)

add_executable(2-cluster
  cluster.cpp
  solver.cpp
  main.cpp)

target_link_libraries(
  2-cluster
  PRIVATE project_warnings
          project_options
          OpenMP::OpenMP_CXX)
