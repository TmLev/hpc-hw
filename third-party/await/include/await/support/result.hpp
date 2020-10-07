#pragma once

// aligned_storage lives here
#include <type_traits>
#include <utility>

#include <await/support/error.hpp>
#include <await/support/function.hpp>

/* References
 *
 * http://joeduffyblog.com/2016/02/07/the-error-model/
 *
 * C++:
 * https://www.boost.org/doc/libs/1_72_0/libs/outcome/doc/html/index.html
 * https://github.com/llvm-mirror/llvm/blob/master/include/llvm/Support/ErrorOr.h
 * https://github.com/facebook/folly/blob/master/folly/Try.h
 * https://abseil.io/docs/cpp/guides/status
 * https://github.com/TartanLlama/expected
 * https://www.youtube.com/watch?v=CGwk3i1bGQI
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0762r0.pdf
 *
 * Rust:
 * https://doc.rust-lang.org/book/ch09-02-recoverable-errors-with-result.html
 */

namespace await {

////////////////////////////////////////////////////////////

namespace detail {

// Low-level storage for value
template <typename T>
class ValueStorage {
  using Storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

 public:
  template <typename... Arguments>
  void Construct(Arguments&&... arguments) {
    new (&storage_) T(std::forward<Arguments>(arguments)...);
  }

  void MoveConstruct(T&& that) {
    new (&storage_) T(std::move(that));
  }

  void CopyConstruct(const T& that) {
    new (&storage_) T(that);
  }

  T* PtrUnsafe() {
    return reinterpret_cast<T*>(&storage_);
  }

  const T* PtrUnsafe() const {
    return reinterpret_cast<const T*>(&storage_);
  }

  void Destroy() {
    PtrUnsafe()->~T();
  }

 private:
  Storage storage_;
};

}  // namespace detail

////////////////////////////////////////////////////////////

template <typename T>
class [[nodiscard]] Result {
 public:
  static_assert(!std::is_reference<T>::value,
                "Reference types are not supported");

  // Static constructors

  template <typename... Arguments>
  static Result Ok(Arguments&&... arguments) {
    Result result;
    result.value_.Construct(std::forward<Arguments>(arguments)...);
    return result;
  }

  static Result Ok(T&& value) {
    return Result(std::move(value));
  }

  static Result Fail(Error error) {
    return Result(std::move(error));
  }

  // Moving

  Result(Result&& that) {
    MoveImpl(std::move(that));
  }

  Result& operator=(Result&& that) {
    DestroyValueIfExist();
    MoveImpl(std::move(that));
    return *this;
  }

  // Copying

  Result(const Result& that) {
    CopyImpl(that);
  }

  Result& operator=(const Result& that) {
    DestroyValueIfExist();
    CopyImpl(that);
    return *this;
  }

  // Dtor

  ~Result() {
    DestroyValueIfExist();
  }

  // Testing

  bool HasError() const {
    return error_.HasError();
  }

  bool IsOk() const {
    return !HasError();
  }

  bool HasValue() const {
    return !HasError();
  }

  // Remove?
  explicit operator bool() const {
    return IsOk();
  }

  void ThrowIfError() {
    error_.ThrowIfError();
  }

  // Ignore result, just check for error
  void ExpectOk() {
    ThrowIfError();
  }

  void Ignore() {
    // No-op
  }

  // Error accessors

  bool MatchErrorCode(int expected) const {
    return error_.GetErrorCode().value() == expected;
  }

  const Error& GetError() const {
    return error_;
  }

  std::error_code GetErrorCode() {
    return error_.GetErrorCode();
  }

  // Value accessors

  // Unsafe value getters, use only after IsOk
  // Behavior is undefined if Result does not contain a value

  T& ValueUnsafe() {
    return *value_.PtrUnsafe();
  }

  const T& ValueUnsafe() const {
    return *value_.PtrUnsafe();
  }

  // Safe value getters
  // Throws if Result does not contain a value

  T& Value() & {
    ThrowIfError();
    return ValueUnsafe();
  }

  const T& Value() const& {
    ThrowIfError();
    return ValueUnsafe();
  }

  T&& Value() && {
    ThrowIfError();
    return std::move(ValueUnsafe());
  }

  // operator * overloads
  // Unsafe: behavior is undefined if Result does not contain a value

  T& operator*() & {
    return ValueUnsafe();
  }

  const T& operator*() const& {
    return ValueUnsafe();
  }

  T&& operator*() && {
    return std::move(ValueUnsafe());
  }

  // operator -> overloads
  // Unsafe: behavior is undefined if Result does not contain a value

  T* operator->() {
    return value_.PtrUnsafe();
  }

  const T* operator->() const {
    return value_.PtrUnsafe();
  }

  // Unwrap rvalue Result automatically
  // Do we need this?

  operator T&&() && {
    return std::move(Value());
  }

 private:
  Result() {
  }

  Result(T&& value) {
    value_.MoveConstruct(std::move(value));
  }

  Result(const T& value) {
    value_.CopyConstruct(value);
  }

  Result(Error error) {
    error_ = std::move(error);
  }

  void MoveImpl(Result&& that) {
    error_ = std::move(that.error_);
    if (that.HasValue()) {
      value_.MoveConstruct(std::move(that.ValueUnsafe()));
    }
  }

  void CopyImpl(const Result& that) {
    error_ = that.error_;
    if (that.HasValue()) {
      value_.CopyConstruct(that.ValueUnsafe());
    }
  }

  void DestroyValueIfExist() {
    if (IsOk()) {
      value_.Destroy();
    }
  }

 private:
  detail::ValueStorage<T> value_;
  Error error_;
};

////////////////////////////////////////////////////////////

template <>
class [[nodiscard]] Result<void> {
 public:
  static Result Ok() {
    return Result{};
  }

  static Result Fail(Error error) {
    return Result(std::move(error));
  }

  Result(Error error) {
    error_ = std::move(error);
  }

  // Testing

  bool HasError() const {
    return error_.HasError();
  }

  bool IsOk() const {
    return !HasError();
  }

  explicit operator bool() const {
    return IsOk();
  }

  void ThrowIfError() {
    error_.ThrowIfError();
  }

  void ExpectOk() {
    ThrowIfError();
  }

  void Ignore() {
    // No-op
  }

  const Error& GetError() const {
    return error_;
  }

  bool MatchErrorCode(int expected) const {
    return GetErrorCode().value() == expected;
  }

  std::error_code GetErrorCode() const {
    return error_.GetErrorCode();
  }

 private:
  Result() = default;

 private:
  Error error_;
};

using Status = Result<void>;

////////////////////////////////////////////////////////////

namespace detail {

class Failure {
 public:
  explicit Failure(std::error_code& error_code) : error_(error_code) {
  }

  explicit Failure(std::exception_ptr exception)
      : error_(std::move(exception)) {
  }

  explicit Failure(Error error) : error_(std::move(error)) {
  }

  Failure(const Failure&) = delete;
  Failure& operator=(const Failure&) = delete;

  Failure(Failure&&) = delete;
  Failure& operator=(Failure&&) = delete;

  template <typename T>
  operator Result<T>() {
    return Result<T>::Fail(error_);
  }

 private:
  Error error_;
};

}  // namespace detail

////////////////////////////////////////////////////////////

namespace make_result {

template <typename T>
Result<T> Ok(T&& value) {
  return Result<T>::Ok(std::move(value));
}

template <typename T>
Result<T> Ok(T& value) {
  return Result<T>::Ok(value);
}

template <typename T>
Result<T> Ok(const T& value) {
  return Result<T>::Ok(value);
}

// Usage: make_result::Ok()
Status Ok() {
  return Status::Ok();
}

detail::Failure CurrentException() {
  return detail::Failure(std::current_exception());
}

// Usage: make_result::Fail(error)
detail::Failure Fail(std::error_code error) {
  return detail::Failure{error};
}

detail::Failure Fail(Error error) {
  return detail::Failure(std::move(error));
}

// Precondition: result.HasError()
template <typename T>
detail::Failure PropagateError(const Result<T>& result) {
  return detail::Failure{result.GetError()};
}

// Convert status code (error or success) to Result
Status ToStatus(std::error_code error) {
  if (error) {
    return Fail(error);
  } else {
    return Ok();
  }
}

template <typename T>
Status JustStatus(const Result<T>& result) {
  if (result.IsOk()) {
    return Ok();
  } else {
    return Status::Fail(result.GetError());
  }
}

template <typename F, typename... Args>
auto Invoke(F&& f, Args&&... args) {
  using T = decltype(f(std::forward<Args>(args)...));
  try {
    // TODO: noexcept ctor
    return Result<T>::Ok(f(std::forward<Args>(args)...));
  } catch (...) {
    return Result<T>::Fail(std::current_exception());
  }
}

// Make result with exception
template <typename E, typename... Args>
detail::Failure Throw(Args&&... args) {
  try {
    throw E(std::forward<Args>(args)...);
  } catch (const E& e) {
    return detail::Failure{std::current_exception()};
  }
}

}  // namespace make_result

}  // namespace await
