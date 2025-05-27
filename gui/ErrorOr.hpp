#pragma once

#include <string>
#include <utility>
#include <variant>

#include "logging/Logger.hpp"

class [[nodiscard]] Error {
public:
  explicit Error(char const *msg, char const *file, int line)
  {
    Log::log(LogLevel::CRIT, file, line) << msg;
    message = std::string(msg);
  }

  explicit Error(std::string msg, char const *file, int line)
    : message(std::move(msg))
  {
    Log::log(LogLevel::CRIT, file, line) << message;
  }

#define Error(msg) Error(msg, __FILE__, __LINE__)

  [[nodiscard]] auto what() const -> const std::string &
  {
    return message;
  }

  [[nodiscard]] auto c_str() const -> const char *
  {
    return message.c_str();
  }

private:
  std::string message;
};

template <typename T> class [[nodiscard]] ErrorOr {
public:
  ErrorOr() = default;

  ErrorOr(const T &value) : result(value) {}

  ErrorOr(T &&value) : result(std::move(value)) {}

  ErrorOr(const Error &error) : result(error) {}

  ErrorOr(Error &&error) : result(std::move(error)) {}

  [[nodiscard]]
  auto has_value() const -> bool
  {
    return std::holds_alternative<T>(result);
  }

  auto value() -> T &
  {
    return std::get<T>(result);
  }

  [[nodiscard]]
  auto value() const -> const T &
  {
    return std::get<T>(result);
  }

  auto error() -> Error &
  {
    return std::get<Error>(result);
  }

  [[nodiscard]]
  auto error() const -> const Error &
  {
    return std::get<Error>(result);
  }

  [[nodiscard]]
  auto is_error() const -> bool
  {
    return std::holds_alternative<Error>(result);
  }

  [[nodiscard]]
  auto operator!() const -> bool
  {
    return is_error();
  }

private:
  std::variant<T, Error> result;
};

struct Nil {};

using MaybeError = ErrorOr<Nil>;

#define TRY(expr)                                                             \
    ({                                                                        \
        _Pragma("GCC diagnostic push");                                       \
        _Pragma("GCC diagnostic ignored \"-Wshadow\"");                       \
        auto&& try_tmp = (expr);                                              \
        if (try_tmp.is_error())                                               \
            return try_tmp.error();                                           \
        _Pragma("GCC diagnostic pop");                                        \
        std::move(try_tmp.value());                                           \
    })

#define MUST(expr, err) if (!((expr))) return Error(err)

#define NotYetImplemented Error("not yet implmented")
