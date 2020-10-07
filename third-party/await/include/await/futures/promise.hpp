#pragma once

#include <await/futures/state.hpp>
#include <await/futures/future.hpp>

#include <await/support/result.hpp>

#include <memory>

namespace await::futures {

using await::make_result::Fail;
using await::make_result::Ok;

template <typename T>
class PromiseBase : public detail::HoldState<T> {
  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

 public:
  PromiseBase() : detail::HoldState<T>(std::make_shared<detail::State<T>>()) {
  }

  // One-shot
  Future<T> MakeFuture() {
    future_extracted_ = true;
    return Future{state_};
  }

  void SetError(Error error) && {
    ReleaseState()->SetResult(Fail(std::move(error)));
  }

  void Set(Result<T> result) && {
    ReleaseState()->SetResult(std::move(result));
  }

 private:
  bool future_extracted_{false};
};

template <typename T>
class Promise : public PromiseBase<T> {
  using Base = PromiseBase<T>;

  using detail::HoldState<T>::ReleaseState;

 public:
  using Base::MakeFuture;
  using Base::Set;
  using Base::SetError;

  void SetValue(T value) && {
    ReleaseState()->SetResult(Ok(std::move(value)));
  }
};

template <>
class Promise<void> : public PromiseBase<void> {
  using Base = PromiseBase<void>;
  using detail::HoldState<void>::ReleaseState;

 public:
  using Base::MakeFuture;
  using Base::Set;
  using Base::SetError;

  void Set() && {
    ReleaseState()->SetResult(Ok());
  }
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using Contract = std::pair<Future<T>, Promise<T>>;

// Usage:
// auto [f, p] = futures::MakeContract<T>();
// https://en.cppreference.com/w/cpp/language/structured_binding

template <typename T>
static Contract<T> MakeContract() {
  Promise<T> p;
  auto f = p.MakeFuture();
  return {std::move(f), std::move(p)};
}

}  // namespace await::futures
