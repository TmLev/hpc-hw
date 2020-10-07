#pragma once

#include <await/futures/promise.hpp>
#include <await/futures/helpers.hpp>

#include <vector>

namespace await::futures::detail {

// TODO: complete future without mutex/spinlock!

//////////////////////////////////////////////////////////////////////

// Generic algorithm
// Does not consider corner cases or immediate completion scenarios

template <template <typename> class Combinator, typename T, typename... Args>
auto Combine(std::vector<Future<T>> futures, Args&&... args) {
  auto combinator = std::make_shared<Combinator<T>>(
      futures.size(), std::forward<Args>(args)...);

  // Do we need this here?
  auto f = combinator->MakeFuture();

  for (size_t i = 0; i < futures.size(); ++i) {
    std::move(futures[i]).Subscribe([combinator, i](Result<T> result) mutable {
      combinator->ProcessInput(std::move(result), i);
    });
  }

  return std::move(f);
}

}  // namespace await::futures::detail
