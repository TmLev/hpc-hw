#pragma once

#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <memory>

#include <await/executors/thread_pool.hpp>

#include <await/executors/helpers.hpp>
#include <await/support/queues.hpp>

namespace await::executors {

class StaticThreadPool final : public IThreadPool {
 public:
  StaticThreadPool(size_t threads) {
    LaunchWorkerThreads(threads);
  }

  ~StaticThreadPool() {
    Shutdown();
  }

  // IExecutor

  void Execute(Task&& task) override {
    WorkCreated();
    tasks_.Put(std::move(task));
  }

  // IThreadPool

  void Join() override {
    joining_.store(true);
    if (work_count_.load() == 0) {
      tasks_.Close();
    }
    JoinWorkers();
  }

  void Shutdown() override {
    tasks_.Shutdown();
    JoinWorkers();
  }

  size_t ExecutedTaskCount() const override {
    return executed_count_.load();
  }

 private:
  void LaunchWorkerThreads(size_t threads) {
    for (size_t i = 0; i < threads; ++i) {
      workers_.emplace_back([this]() { WorkerRoutine(); });
    }
  }

  void WorkerRoutine() {
    while (auto task = tasks_.Take()) {
      SafelyExecuteHere(task.value());
      WorkCompleted();
      executed_count_.fetch_add(1);
    }
  }

  void WorkCreated() {
    work_count_.fetch_add(1);
  }

  void WorkCompleted() {
    if (work_count_.fetch_sub(1) == 1 && joining_.load()) {
      tasks_.Close();
    }
  }

  void JoinWorkers() {
    for (auto& worker : workers_) {
      worker.join();
    }
    workers_.clear();
  }

 private:
  support::MPMCBlockingQueue<Task> tasks_;
  std::vector<std::thread> workers_;
  std::atomic<size_t> work_count_{0};
  std::atomic<size_t> executed_count_{0};
  std::atomic<bool> joining_{false};
};

// Fixed-size pool of threads + unbounded blocking queue
IThreadPoolPtr MakeStaticThreadPool(size_t threads) {
  return std::make_shared<StaticThreadPool>(threads);
}

}  // namespace await::executors
