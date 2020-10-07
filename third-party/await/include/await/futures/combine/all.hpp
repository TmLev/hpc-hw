#pragma once

#include <await/futures/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>
#include <await/futures/combine/detail/traits.hpp>
#include <await/futures/helpers.hpp>

#include <await/support/thread_spinlock.hpp>

#include <vector>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// All

namespace detail {

template <typename T>
class AllCombinator {
  using Traits = CombinatorTraits<T>;
  using ValuesVector = typename Traits::ValuesVector;
  using ValuesVectorBuilder = typename Traits::ValuesVectorBuilder;

 public:
  AllCombinator(size_t num_futures) : num_remained_(num_futures) {
    values_.Reserve(num_futures);  // Be optimistic
  }

  ~AllCombinator() {
    if (!failed_) {
      std::move(values_).Complete(std::move(promise_));
    }
  }

  void ProcessInput(Result<T> result, size_t /*index*/) {
    std::lock_guard guard(mutex_);

    if (result.HasError()) {
      if (!std::exchange(failed_, true)) {
        std::move(promise_).SetError(std::move(result.GetError()));
      }
      return;
    }

    // result.IsOk() == true
    values_.Add(std::move(result));
  }

  Future<ValuesVector> MakeFuture() {
    return std::move(promise_.MakeFuture());
  }

 private:
  support::ThreadSpinLock mutex_;
  size_t num_remained_{0};
  bool failed_{false};
  ValuesVectorBuilder values_;
  Promise<ValuesVector> promise_;
};

}  // namespace detail

// All values / first error
template <typename T>
auto All(std::vector<Future<T>> inputs) {
  if (inputs.empty()) {
    return detail::CombinatorTraits<T>::MakeEmptyOutput();
  }
  return detail::Combine<detail::AllCombinator>(std::move(inputs));
}

}  // namespace await::futures
