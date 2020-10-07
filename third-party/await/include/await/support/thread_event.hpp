#pragma once

#include <mutex>

namespace await::support {

class ThreadOneShotEvent {
 public:
  void Set() {
    std::lock_guard guard(mutex_);
    set_ = true;
    set_cond_.notify_all();
  }

  void Await() {
    std::unique_lock lock(mutex_);
    while (!set_) {
      set_cond_.wait(lock);
    }
  }

 private:
  std::mutex mutex_;
  std::condition_variable set_cond_;
  bool set_{false};
};

}  // namespace await::support
