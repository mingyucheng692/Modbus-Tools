/**
 * @file TrContext.h
 * @brief Template-based i18n translation helper wrapping QCoreApplication::translate.
 *
 * Replaces repetitive trXxx wrapper functions with a single template.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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