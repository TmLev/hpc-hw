#pragma once

#include <await/futures/future.hpp>
#include <await/futures/promise.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// Future<T> -> Future<void>

template <typename T>
Future<void> JustStatus(Future<T>&& f) {
  auto [jsf, jsp] = MakeContract<void>();
  std::move(f).Subscribe([jsp = std::move(jsp)](Result<T> result) mutable {
    if (result.IsOk()) {
      std::move(jsp).Set();
    } else {
      // Propagate error
      std::move(jsp).SetError(result.GetError());
    }
  });
  return std::move(jsf);
}

//////////////////////////////////////////////////////////////////////

template <typename T>
Future<T> MakeValue(T&& value) {
  auto [f, p] = MakeContract<T>();
  std::move(p).SetValue(std::forward<T>(value));
  return std::move(f);
}

//////////////////////////////////////////////////////////////////////

Future<void> MakeCompletedVoid() {
  auto [f, p] = MakeContract<void>();
  std::move(p).Set();
  return std::move(f);
}

//////////////////////////////////////////////////////////////////////

namespace detail {

class Failure {
 public:
  explicit Failure(Error&& error) : error_(std::move(error)) {
  }

  // Non-copyable
  Failure(const Failure& that) = delete;
  Failure& operator=(const Failure& that) = delete;

  template <typename T>
  Future<T> As() && {
    auto [f, p] = MakeContract<T>();
    std::move(p).SetError(std::move(error_));
    return std::move(f);
  }

  template <typename T>
  operator Future<T>() && {
    return std::move(*this).As<T>();
  }

 private:
  Error error_;
};

}  // namespace detail

// Usage:
// 1) Future<T> f = MakeError(e);
// 2) auto f = MakeError(e).As<T>();

detail::Failure MakeError(Error&& error) {
  return detail::Failure{std::move(error)};
}

//////////////////////////////////////////////////////////////////////

template <typename F, typename T>
Future<T> RecoverWith(Future<T>&& f, F&& fallback) {
  auto [rwf, rwp] = MakeContract<T>();

  auto recover = [p = std::move(rwp), fallback = std::forward<F>(fallback)](
                     Result<T> result) mutable {
    if (result.IsOk()) {
      std::move(p).Set(std::move(result));
    } else {
      // Fallback
      Future<T> ff = fallback(result.GetError());
      std::move(ff).Subscribe([p = std::move(p)](Result<T> result) mutable {
        std::move(p).Set(std::move(result));
      });
    }
  };
  std::move(f).Subscribe(std::move(recover));

  return std::move(rwf);
}

}  // namespace await::futures
