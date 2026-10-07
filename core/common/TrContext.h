// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QCoreApplication>

/**
 * @brief TrContext template wraps QCoreApplication::translate with a compile-time context string.
 *
 * Usage:
 *   constexpr char kMyCtx[] = "MyContext";
 *   TrContext<kMyCtx>::tr("Hello");
 *   TrContext<kMyCtx>::tr("Hello %1").arg(world);
 */
template <const char* Context>
struct TrContext {
    static QString tr(const char* sourceText,
                      const char* disambiguation = nullptr,
                      int n = -1)
    {
        return QCoreApplication::translate(Context, sourceText, disambiguation, n);
    }
};
