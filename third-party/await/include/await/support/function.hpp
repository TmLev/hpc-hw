#pragma once

#include <await/support/function_impl.hpp>

// https://github.com/Naios/function2
// Commit: 3a0746bf5f601dfed05330aefcb6854354fce07d

namespace await::support {

template <typename Signature>
using UniqueFunction = fu2::unique_function<Signature>;

}  // namespace await::support
