#pragma once

#include <atomic>
#include <thread>

namespace await::support {

//////////////////////////////////////////////////////////////////////

// Relax in favour of the CPU owning the lock

// https://c9x.me/x86/html/file_module_x86_id_232.html
// https://aloiskraus.wordpress.com/2018/06/16/why-skylakex-cpus-are-sometimes-50-slower-how-intel-has-broken-existing-code/

inline void SpinLockPause() {
  asm volatile("pause\n" : : : "memory");
}

//////////////////////////////////////////////////////////////////////

class ThreadSpinWait {
  static const size_t kYieldThreshold = 159;

 public:
  void operator()() {
    if (spins_++ > kYieldThreshold) {
      Yield();
    } else {
      SpinLockPause();
    }
  }

 private:
  static void Yield() {
    std::this_thread::yield();
  }

 private:
  size_t spins_{0};
};

//////////////////////////////////////////////////////////////////////

class ThreadSpinLock {
  using ScopeGuard = std::lock_guard<ThreadSpinLock>;

 public:
  void Lock() {
    // TODO: TaTAS
    ThreadSpinWait spin_wait;
    while (locked_.exchange(true)) {
      spin_wait();
    }
  }

  bool TryLock() {
    return !locked_.exchange(true);
  }

  void Unlock() {
    locked_.store(false);
  }

  ScopeGuard Guard() {
    return ScopeGuard{*this};
  }

  // Lockable

  void lock() {
    Lock();
  }

  bool try_lock() {
    return TryLock();
  }

  void unlock() {
    Unlock();
  }

 private:
  std::atomic<bool> locked_{false};
};

}  // namespace await::support
