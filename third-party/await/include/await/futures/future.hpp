#pragma once

#include <await/futures/state.hpp>

namespace await::futures {

using executors::IExecutorPtr;

//////////////////////////////////////////////////////////////////////

template <typename T>
class Promise;

template <typename T>
class PromiseBase;

//////////////////////////////////////////////////////////////////////

template <typename T>
class Future : public detail::HoldState<T> {
  friend class Promise<T>;
  friend class PromiseBase<T>;

  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::HasState;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

  // Subscribe
  // Result<T> -> void
  using Callback = typename detail::State<T>::Callback;

  // Then
  // Result<T> -> U
  template <typename U>
  using Continuation = await::support::UniqueFunction<U(Result<T>)>;

 public:
  // True if this future has a shared state
  // False if result has already been consumed
  // 1) synchronously via GetReadyResult/GetResult or
  // 2) via Subscribe
  bool IsValid() const {
    return HasState();
  }

  // Non-blocking
  // True if this future has result in its shared state
  bool IsReady() const {
    CheckState();
    return state_->HasResult();
  }

  // Non-blocking
  // Pre-condition: IsReady() == true
  Result<T> GetReadyResult() && {
    return ReleaseState()->GetReadyResult();
  }

  // Blocking
  // Await future result
  Result<T> GetResult() &&;

  // Asynchronous API

  // Set executor for asynchronous callback
  Future<T> Via(executors::IExecutorPtr e) && {
    state_->SetExecutor(std::move(e));
    return std::move(*this);
  }

  // Consume result with asynchronous callback
  // Post-condition: IsValid() == false
  void Subscribe(Callback callback) && {
    ReleaseState()->SetCallback(std::move(callback));
  }

  // Continuations

  template <typename F>
  auto Then(F&& f) &&;

  // Synchronous continuation

  // Future<T> -> Continuation U(Result<T>) -> Future<U>
  template <typename U>
  Future<U> Then(Continuation<U> f) &&;

  // Static constructors

  static Future<T> Invalid() {
    return Future(nullptr);
  }

 private:
  Future(detail::StateRef<T> state) : detail::HoldState<T>(std::move(state)) {
  }
};

}  // namespace await::futures

#define FUTURE_IMPL
#include <await/futures/future_impl.hpp>
#undef FUTURE_IMPL
