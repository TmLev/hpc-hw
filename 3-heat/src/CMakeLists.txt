find_package(MPI REQUIRED)

add_executable(3-heat
  main.cpp)

target_link_libraries(
  3-heat
  PRIVATE project_warnings
          project_options
          MPI::MPI_CXX)
