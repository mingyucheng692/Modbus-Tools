/**
 * @file Win32Encoding.h
 * @brief Shared UTF-8 <-> UTF-16 conversion helpers for the Win32 updater.
 */

#pragma once

#include <string>
#include <Windows.h>

namespace updater::win32 {

/// Converts UTF-8 string to UTF-16 wstring (Win32 boundary conversion).
[[nodiscard]] inline std::wstring utf8ToWide(const std::string& utf8)
{
    if (utf8.empty()) {
        return {};
    }
    const int required = MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                                             static_cast<int>(utf8.size()), nullptr, 0);
    if (required <= 0) {
        return {};
    }
    std::wstring wide(required, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
                        wide.data(), required);
    return wide;
}

/// Converts UTF-16 wstring to UTF-8 string (Win32 boundary conversion).
[[nodiscard]] inline std::string wideToUtf8(const std::wstring& wide)
{
    if (wide.empty()) {
        return {};
    }
    const int required = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                             static_cast<int>(wide.size()),
                                             nullptr, 0, nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string utf8(required, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
                       utf8.data(), required, nullptr, nullptr);
    return utf8;
}

} // namespace updater::win32
