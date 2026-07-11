/**
 * @file Result.h
 * @brief Generic Result<T,E> type for success/error return values.
 *
 * Replaces the ad-hoc result structs (PduBuildResult, BuildResult, etc.)
 * with a single template. In a -fno-exceptions project, callers must check
 * isOk() before calling value(); misuse triggers std::terminate to avoid
 * undefined behavior from dereferencing an empty optional.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <optional>
#include <exception>
#include <utility>

namespace core::result {

template <typename T, typename E>
class Result {
public:
    [[nodiscard]] static Result ok(T value) {
        Result r;
        r.value_.emplace(std::move(value));
        return r;
    }

    [[nodiscard]] static Result fail(E error) {
        Result r;
        r.error_ = std::move(error);
        return r;
    }

    [[nodiscard]] bool isOk() const { return value_.has_value(); }

    [[nodiscard]] const T& value() const {
        if (!value_.has_value()) {
            std::terminate();
        }
        return *value_;
    }

    [[nodiscard]] T& value() {
        if (!value_.has_value()) {
            std::terminate();
        }
        return *value_;
    }

    [[nodiscard]] const E& error() const { return error_; }

private:
    std::optional<T> value_;
    E error_{};
};

} // namespace core::result
