#pragma once

#include <await/executors/executor.hpp>
#include <await/executors/helpers.hpp>

namespace await::executors {

class InlineExecutor : public IExecutor {
 public:
  void Execute(Task&& task) override {
    SafelyExecuteHere(task);
  }
};

class Instance {
 public:
  Instance() : e_(std::make_shared<InlineExecutor>()) {
  }

  IExecutorPtr Get() {
    return e_;
  }

 private:
  IExecutorPtr e_;
};

IExecutorPtr GetInlineExecutor() {
  return std::make_shared<InlineExecutor>();
}

}  // namespace await::executors
