#pragma once

#include <await/executors/task.hpp>

namespace await::executors {

void SafelyExecuteHere(Task& task) {
  try {
    task();
  } catch (...) {
    // ¯\_(ツ)_/¯
  }
}

}  // namespace await::executors
