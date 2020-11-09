cmake_minimum_required(VERSION 3.12)

project(
  hpc-hw
  VERSION 0.1.0
  LANGUAGES CXX)

# Public options
option(ENABLE_THREAD_SANITIZER "Enable thread sanitizer for project" OFF)

include(cmake/Settings.cmake)
include(cmake/Ccache.cmake)

add_library(project_options INTERFACE)
if (ENABLE_THREAD_SANITIZER)
  message(STATUS "thread sanitizer is enabled")
  include(cmake/Sanitizers.cmake)
  enable_thread_sanitizer(project_options)
else()
  message(STATUS "thread sanitizer is disabled")
endif()

add_library(project_warnings INTERFACE)
include(cmake/Warnings.cmake)
set_project_warnings(project_warnings)

add_subdirectory(third-party)

add_subdirectory(1-knapsack)
add_subdirectory(2-cluster)
