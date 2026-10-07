// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QCoreApplication>

// Role: Compile-time translation wrapper binding QCoreApplication::translate to a fixed context string.
// Thread-safety: Thread-safe (relies on Qt's reentrant QCoreApplication::translate).
// Ownership: Pure template utility struct; non-instantiable statically accessed wrapper.
template <const char* Context>
struct TrContext {
    static QString tr(const char* sourceText,
                      const char* disambiguation = nullptr,
                      int n = -1)
    {
        return QCoreApplication::translate(Context, sourceText, disambiguation, n);
    }
};
