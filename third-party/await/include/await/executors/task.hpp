#pragma once

#include <await/support/function.hpp>

namespace await::executors {

using Task = await::support::UniqueFunction<void()>;

}  // namespace await::executors
