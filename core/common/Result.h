// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <variant>
#include <optional>
#include <type_traits>
#include <utility>

namespace common {

// Role: Tag wrapper representing an unexpected (error) value for monadic result construction.
// Thread-safety: Thread-safe as a value type for independent instances.
// Ownership: Value object passed by copy or move.
template <typename E>
struct Unexpected {
    E error;

    explicit constexpr Unexpected(const E& err) : error(err) {}
    explicit constexpr Unexpected(E&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        : error(std::move(err)) {}
};

template <typename E>
constexpr Unexpected<std::decay_t<E>> makeUnexpected(E&& err) {
    return Unexpected<std::decay_t<E>>(std::forward<E>(err));
}

// Role: Monadic Result container holding either a success value T or an error E (-fno-exceptions).
// Thread-safety: Thread-safe as a value type for independent instances.
// Ownership: Value object passed by copy or move.
template <typename T, typename E>
class [[nodiscard]] Result {
public:
    using value_type = T;
    using error_type = E;

    // Value constructors
    constexpr Result(const T& val) : storage_(val) {}
    constexpr Result(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
        : storage_(std::move(val)) {}

    // Error constructor
    constexpr Result(Unexpected<E> unexp) noexcept(std::is_nothrow_move_constructible_v<E>)
        : storage_(std::move(unexp.error)) {}

    [[nodiscard]] constexpr bool isOk() const noexcept {
        return std::holds_alternative<T>(storage_);
    }

    [[nodiscard]] constexpr bool isErr() const noexcept {
        return std::holds_alternative<E>(storage_);
    }

    constexpr explicit operator bool() const noexcept {
        return isOk();
    }

    [[nodiscard]] const T& value() const& {
        return std::get<T>(storage_);
    }

    [[nodiscard]] T& value() & {
        return std::get<T>(storage_);
    }

    [[nodiscard]] T&& value() && {
        return std::get<T>(std::move(storage_));
    }

    [[nodiscard]] const E& error() const& {
        return std::get<E>(storage_);
    }

    [[nodiscard]] E& error() & {
        return std::get<E>(storage_);
    }

    [[nodiscard]] E&& error() && {
        return std::get<E>(std::move(storage_));
    }

    [[nodiscard]] const T* valuePtr() const noexcept {
        return isOk() ? &std::get<T>(storage_) : nullptr;
    }

    [[nodiscard]] T* valuePtr() noexcept {
        return isOk() ? &std::get<T>(storage_) : nullptr;
    }

    [[nodiscard]] const E* errorPtr() const noexcept {
        return isErr() ? &std::get<E>(storage_) : nullptr;
    }

    [[nodiscard]] E* errorPtr() noexcept {
        return isErr() ? &std::get<E>(storage_) : nullptr;
    }

    template <typename U>
    [[nodiscard]] T valueOr(U&& defaultValue) const& {
        return isOk() ? std::get<T>(storage_) : static_cast<T>(std::forward<U>(defaultValue));
    }

    template <typename U>
    [[nodiscard]] T valueOr(U&& defaultValue) && {
        return isOk() ? std::get<T>(std::move(storage_)) : static_cast<T>(std::forward<U>(defaultValue));
    }

    static Result<T, E> ok(T value) {
        return Result<T, E>(std::move(value));
    }

    static Result<T, E> err(E error) {
        return Result<T, E>(Unexpected<E>(std::move(error)));
    }

private:
    std::variant<T, E> storage_;
};

// Role: Partial specialization of Result for void value type representing success-or-error.
// Thread-safety: Thread-safe as a value type for independent instances.
// Ownership: Value object passed by copy or move.
template <typename E>
class [[nodiscard]] Result<void, E> {
public:
    using value_type = void;
    using error_type = E;

    constexpr Result() noexcept : error_(std::nullopt) {}
    constexpr Result(Unexpected<E> unexp) noexcept(std::is_nothrow_move_constructible_v<E>)
        : error_(std::move(unexp.error)) {}

    [[nodiscard]] constexpr bool isOk() const noexcept {
        return !error_.has_value();
    }

    [[nodiscard]] constexpr bool isErr() const noexcept {
        return error_.has_value();
    }

    constexpr explicit operator bool() const noexcept {
        return isOk();
    }

    [[nodiscard]] const E& error() const& {
        return *error_;
    }

    [[nodiscard]] E& error() & {
        return *error_;
    }

    [[nodiscard]] E&& error() && {
        return *std::move(error_);
    }

    [[nodiscard]] const E* errorPtr() const noexcept {
        return error_.has_value() ? &*error_ : nullptr;
    }

    [[nodiscard]] E* errorPtr() noexcept {
        return error_.has_value() ? &*error_ : nullptr;
    }

    static Result<void, E> ok() {
        return Result<void, E>();
    }

    static Result<void, E> err(E error) {
        return Result<void, E>(Unexpected<E>(std::move(error)));
    }

private:
    std::optional<E> error_;
};

} // namespace common
