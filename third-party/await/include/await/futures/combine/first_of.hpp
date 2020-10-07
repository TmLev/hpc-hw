#pragma once

#include <await/futures/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>
#include <await/futures/helpers.hpp>

#include <atomic>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// FirstOf

namespace detail {

template <typename T>
class FirstOfCombinator {
 public:
  FirstOfCombinator(size_t num_futures) : num_futures_(num_futures) {
  }

  void ProcessInput(Result<T> result, size_t /*index*/) {
    if (result.IsOk()) {
      if (!completed_.exchange(true)) {
        std::move(promise_).Set(std::move(result));
      }
    } else {
      // Error
      if (errors_.fetch_add(1) + 1 == num_futures_) {
        // Last error
        std::move(promise_).SetError(result.GetError());
      }
    }
  }

  Future<T> MakeFuture() {
    return std::move(promise_.MakeFuture());
  }

 private:
  const size_t num_futures_;
  std::atomic<size_t> errors_{0};
  std::atomic<bool> completed_{false};
  Promise<T> promise_;
};

}  // namespace detail

// First value or last error
template <typename T>
Future<T> FirstOf(std::vector<Future<T>> inputs) {
  if (inputs.empty()) {
    abort();
  }
  return detail::Combine<detail::FirstOfCombinator>(std::move(inputs));
}

}  // namespace await::futures
