#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <vector>

namespace await::support {

//////////////////////////////////////////////////////////////////////

// Multi-producer/multi-consumer unbounded blocking queue

template <typename T>
class MPMCBlockingQueue {
 public:
  // Returns false iff queue is closed / shutted down
  bool Put(T item) {
    std::lock_guard lock(mutex_);
    if (closed_) {
      return false;
    }
    items_.push_back(std::move(item));
    not_empty_.notify_one();
    return true;
  }

  // Await and take next item
  // Returns false iff queue is both 1) drained and 2) closed
  std::optional<T> Take() {
    std::unique_lock lock(mutex_);
    while (items_.empty() && !closed_) {
      not_empty_.wait(lock);
    }
    if (!items_.empty()) {
      T value = std::move(items_.front());
      items_.pop_front();
      return std::move(value);
    } else {
      return {};
    }
  }

  // Close queue for producers
  void Close() {
    std::lock_guard lock(mutex_);
    closed_ = true;
    not_empty_.notify_all();
  }

  // Close queue for producers and consumers,
  // discard existing items
  void Shutdown() {
    std::lock_guard lock(mutex_);
    items_.clear();
    closed_ = true;
    not_empty_.notify_all();
  }

 private:
  std::deque<T> items_;
  bool closed_{false};
  std::mutex mutex_;
  std::condition_variable not_empty_;
};

//////////////////////////////////////////////////////////////////////

// Multi-producer/single-consumer (MPSC) unbounded lock-free queue

template <typename T>
class MPSCLockFreeQueue {
  struct Node {
    Node(T item, Node* next) : item_(std::move(item)), next_(next) {
    }
    T item_;
    Node* next_;
  };

 public:
  ~MPSCLockFreeQueue() {
    TakeAll();
  }

  void Put(T item) {
    auto node = new Node{std::move(item), top_.load()};

    while (!top_.compare_exchange_weak(node->next_, node)) {
    }
  }

  std::vector<T> TakeAll() {
    auto top = top_.exchange(nullptr);
    return ReverseToVector(top);
  }

  bool IsEmpty() const {
    return top_.load() == nullptr;
  }

 private:
  static std::vector<T> ReverseToVector(Node* node) {
    std::vector<T> items;
    while (node != nullptr) {
      items.push_back(std::move(node->item_));
      auto to_delete = std::exchange(node, node->next_);
      delete to_delete;
    }
    std::reverse(items.begin(), items.end());
    return items;
  }

 private:
  std::atomic<Node*> top_{nullptr};
};

}  // namespace await::support
