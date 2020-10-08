add_executable(01-knapsack
  knapsack.cpp
  main.cpp)
target_link_libraries(
  01-knapsack
  PRIVATE project_warnings
          project_options
          await
)
