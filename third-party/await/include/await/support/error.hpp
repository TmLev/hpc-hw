#pragma once

#include <exception>
#include <system_error>
#include <variant>

namespace await {

class Error {
 public:
  Error() : error_(std::monostate{}) {
  }

  Error(std::exception_ptr e) : error_(std::move(e)) {
  }

  Error(std::error_code e) : error_(std::move(e)) {
  }

  bool HasException() const {
    return std::holds_alternative<std::exception_ptr>(error_);
  }

  bool HasErrorCode() const {
    return std::holds_alternative<std::error_code>(error_);
  }

  bool HasError() const {
    return HasException() || HasErrorCode();
  }

  std::error_code GetErrorCode() const {
    return std::get<std::error_code>(error_);
  }

  // HasErrorCode -> std::system_error
  void ThrowIfError() {
    if (HasException()) {
      std::rethrow_exception(std::get<std::exception_ptr>(error_));
    } else if (HasErrorCode()) {
      throw std::system_error(std::get<std::error_code>(error_));
    }
  }

 private:
  std::variant<std::monostate, std::exception_ptr, std::error_code> error_;
};

}  // namespace await
