add_executable(1-knapsack
  context.cpp
  knapsack.cpp
  main.cpp)

target_link_libraries(
  1-knapsack
  PRIVATE project_warnings
          project_options
          await
          pthread)
