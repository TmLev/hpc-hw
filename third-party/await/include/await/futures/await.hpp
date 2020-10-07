#pragma once

#include <await/futures/future.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// Fiber/thread-friendly await procedure
template <typename T>
Result<T> Await(Future<T>&& future) {
  return std::move(future).GetResult();
}

}  // namespace await::futures
