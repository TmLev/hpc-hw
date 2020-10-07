#pragma once

#include <await/futures/promise.hpp>
#include <await/futures/helpers.hpp>

#include <vector>

namespace await::futures::detail {

//////////////////////////////////////////////////////////////////////

// T != void

template <typename T>
struct CombinatorTraits {
  using ValuesVector = std::vector<T>;

  static Future<ValuesVector> MakeEmptyOutput() {
    return MakeValue(std::vector<T>{});
  }

  class ValuesVectorBuilder {
   public:
    void Reserve(size_t slots) {
      values_.reserve(slots);
    }

    // Pre-condition: value.IsOk() == true
    void Add(Result<T> value) {
      WHEELS_VERIFY(value.IsOk(), "Value expected");
      values_.push_back(std::move(*value));
    }

    void Complete(Promise<std::vector<T>>&& promise) && {
      std::move(promise).SetValue(std::move(values_));
    }

   private:
    std::vector<T> values_;
  };
};

//////////////////////////////////////////////////////////////////////

// void

template <>
struct CombinatorTraits<void> {
  using ValuesVector = void;

  static Future<ValuesVector> MakeEmptyOutput() {
    return MakeCompletedVoid();
  }

  struct ValuesVectorBuilder {
    void Reserve(size_t /*slots*/) {
      // Nop
    }

    // Pre-condition: value.IsOk() == true
    void Add(Result<void> value) {
      // Nop
    }

    void Complete(Promise<void>&& promise) {
      std::move(promise).Set();
    }
  };
};

}  // namespace await::futures::detail
