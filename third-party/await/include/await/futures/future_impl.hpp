#ifndef FUTURE_IMPL
#error Do not include this file directly
#endif

#include <await/support/thread_event.hpp>

#include <optional>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// GetResult

namespace detail {

template<typename T>
class Awaiter {
 public:
  Awaiter(Future<T> &&f) : future_(std::move(f)) {
  }

  Result<T> Await() && {
    std::move(future_).Subscribe([this](Result<T> result) {
      result_.emplace(std::move(result));
      completed_.Set();
    });
    completed_.Await();
    return std::move(*result_);
  }

 private:
  Future<T> future_;
  support::ThreadOneShotEvent completed_;
  std::optional<Result<T>> result_;
};

}  // namespace detail

template <typename T>
Result<T> Future<T>::GetResult() && {
  detail::Awaiter<T> awaiter(std::move(*this));
  return std::move(awaiter).Await();
}

//////////////////////////////////////////////////////////////////////

// Then

// Impl note: dispatch continuations by return value
template <typename T>
template <typename F>
auto Future<T>::Then(F&& f) && {
  using U = decltype(f(std::declval<Result<T>>()));

  return std::move(*this).Then(Continuation<U>(std::forward<F>(f)));
}

template <typename T>
template <typename U>
Future<U> Future<T>::Then(Continuation<U> f) && {
  Promise<U> pu;
  auto fu = pu.MakeFuture();

  auto callback = [f = std::move(f), pu = std::move(pu)](Result<T> result) mutable {
    // Apply f to Result<T>
    auto next_result = await::make_result::Invoke(f, std::move(result));
    std::move(pu).Set(std::move(next_result));
  };

  std::move(*this).Subscribe(std::move(callback));

  return std::move(fu);
}

}  // namespace await::futures
