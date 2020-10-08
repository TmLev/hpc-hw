add_executable(1-knapsack
  knapsack.cpp
  main.cpp)
target_link_libraries(
  1-knapsack
  PRIVATE project_warnings
          project_options
          await
)
