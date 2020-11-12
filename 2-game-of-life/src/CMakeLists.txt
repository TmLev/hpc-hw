find_package(MPI REQUIRED)

add_executable(2-game-of-life
  board.cpp
  worker.cpp
  world.cpp
  main.cpp)

target_link_libraries(
  2-game-of-life
  PRIVATE project_warnings
          project_options
          MPI::MPI_CXX)
