#pragma once

#include <await/executors/executor.hpp>
#include <await/executors/helpers.hpp>
#include <await/executors/inline.hpp>

#include <await/support/function.hpp>
#include <await/support/result.hpp>

#include <atomic>
#include <optional>

namespace await::futures {

using executors::IExecutorPtr;

namespace detail {

//////////////////////////////////////////////////////////////////////

// State shared between Promise and Future

template <typename T>
class State {
  enum class EState {
    Initial = 0,
    CallbackSet = 1,
    ResultSet = 2,
    ResultConsumed = 3,  // Terminal state
  };

 public:
  using Callback = await::support::UniqueFunction<void(Result<T>)>;

 public:
  State() : executor_(executors::GetInlineExecutor()) {
  }

  void SetResult(Result<T>&& result) {
    result_.emplace(std::move(result));

    switch (state_.exchange(EState::ResultSet)) {
      case EState::CallbackSet:
        InvokeCallback();
        break;
      case EState::Initial:
        // Do nothing
        break;
      default:
        abort();
    }
  }

  bool HasResult() const {
    return state_.load() == EState::ResultSet;
  }

  Result<T> GetReadyResult() {
    return std::move(*result_);
  }

  void SetExecutor(IExecutorPtr e) {
    executor_ = std::move(e);
  }

  void SetCallback(Callback callback) {
    callback_ = std::move(callback);

    switch (state_.exchange(EState::CallbackSet)) {
      case EState::Initial:
        // Do nothing
        break;
      case EState::ResultSet:
        InvokeCallback();
        break;
      default:
        abort();
    }
  }

 private:
  void InvokeCallback() {
    state_.store(EState::ResultConsumed);
    executor_->Execute([result = std::move(*result_),
                        callback = std::move(callback_)]() mutable {
      callback(std::move(result));
    });
  }

 private:
  std::atomic<EState> state_{EState::Initial};
  std::optional<Result<T>> result_;
  Callback callback_;
  IExecutorPtr executor_;
};

//////////////////////////////////////////////////////////////////////

// Common base for Promise and Future

template <typename T>
using StateRef = std::shared_ptr<State<T>>;

template <typename T>
class HoldState {
 protected:
  HoldState(StateRef<T> state) : state_(std::move(state)) {
  }

  // Movable
  HoldState(HoldState&& that) = default;
  HoldState& operator=(HoldState&& that) = default;

  // Non-copyable
  HoldState(const HoldState& that) = delete;
  HoldState& operator=(const HoldState& that) = delete;

  StateRef<T> ReleaseState() {
    CheckState();
    return std::move(state_);
  }

  bool HasState() const {
    return (bool)state_;
  }

  void CheckState() const {
    assert(state_);
  }

 protected:
  StateRef<T> state_;
};

}  // namespace detail

}  // namespace await::futures
