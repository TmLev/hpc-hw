cmake_minimum_required(VERSION 3.12)

project(
  hpc-hw
  VERSION 0.1.0
  LANGUAGES CXX)

include(cmake/Settings.cmake)
include(cmake/Ccache.cmake)

add_library(project_options INTERFACE)
include(cmake/Sanitizers.cmake)
enable_thread_sanitizer(project_options)

add_library(project_warnings INTERFACE)
include(cmake/Warnings.cmake)
set_project_warnings(project_warnings)

add_subdirectory(third-party)

add_subdirectory(1-knapsack)
