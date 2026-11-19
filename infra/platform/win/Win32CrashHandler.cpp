// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "infra/platform/CrashHandler.h"

#if defined(_WIN32)

#include <windows.h>
#include <DbgHelp.h>
#include <crtdbg.h>
#include <exception>
#include <cstdio>
#include <cwchar>

namespace infra::platform {

namespace {

// Static buffer for log directory wide path, initialized once during install().
// NO heap allocations are permitted inside the crash handlers.
wchar_t s_logDirW[MAX_PATH] = {0};
bool s_installed = false;

void formatCrashTimestamp(wchar_t* out, size_t maxLen) noexcept {
    SYSTEMTIME st;
    GetLocalTime(&st);
    swprintf_s(out, maxLen, L"%04u%02u%02u-%02u%02u%02u",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

void writeCrashTextLog(const wchar_t* txtPath, EXCEPTION_POINTERS* ep) noexcept {
    HANDLE hFile = CreateFileW(txtPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    char buffer[1024];
    DWORD code = 0;
    void* addr = nullptr;
    if (ep && ep->ExceptionRecord) {
        code = ep->ExceptionRecord->ExceptionCode;
        addr = ep->ExceptionRecord->ExceptionAddress;
    }

    int len = sprintf_s(buffer, sizeof(buffer),
        "=== CRASH REPORT ===\r\n"
        "Process ID: %lu\r\n"
        "Thread ID:  %lu\r\n"
        "Exception Code:    0x%08lX\r\n"
        "Exception Address: 0x%p\r\n",
        GetCurrentProcessId(),
        GetCurrentThreadId(),
        code,
        addr);

    if (len > 0) {
        DWORD written = 0;
        WriteFile(hFile, buffer, static_cast<DWORD>(len), &written, nullptr);
    }
    CloseHandle(hFile);
}

void writeMinidump(const wchar_t* dmpPath, EXCEPTION_POINTERS* ep) noexcept {
    HANDLE hFile = CreateFileW(dmpPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    MiniDumpWriteDump(GetCurrentProcess(),
                      GetCurrentProcessId(),
                      hFile,
                      MiniDumpNormal,
                      ep ? &mei : nullptr,
                      nullptr,
                      nullptr);

    CloseHandle(hFile);
}

LONG WINAPI UnhandledCrashFilter(EXCEPTION_POINTERS* ep) noexcept {
    wchar_t timeBuf[32];
    formatCrashTimestamp(timeBuf, 32);

    const DWORD pid = GetCurrentProcessId();

    wchar_t dmpPath[MAX_PATH];
    swprintf_s(dmpPath, MAX_PATH, L"%s\\crash_%s_%lu.dmp", s_logDirW, timeBuf, pid);

    wchar_t txtPath[MAX_PATH];
    swprintf_s(txtPath, MAX_PATH, L"%s\\crash_%s_%lu.txt", s_logDirW, timeBuf, pid);

    writeMinidump(dmpPath, ep);
    writeCrashTextLog(txtPath, ep);

    return EXCEPTION_CONTINUE_SEARCH;
}

void InvalidParameterHandler(const wchar_t* expression,
                             const wchar_t* function,
                             const wchar_t* file,
                             unsigned int line,
                             uintptr_t /*reserved*/) noexcept {
    (void)expression; (void)function; (void)file; (void)line;
    __try {
        RaiseException(0xE0000001, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    } __except (UnhandledCrashFilter(GetExceptionInformation())) {
    }
    std::abort();
}

void PureCallHandler() noexcept {
    __try {
        RaiseException(0xE0000002, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    } __except (UnhandledCrashFilter(GetExceptionInformation())) {
    }
    std::abort();
}

void TerminateHandler() noexcept {
    __try {
        RaiseException(0xE0000003, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    } __except (UnhandledCrashFilter(GetExceptionInformation())) {
    }
    std::abort();
}

} // namespace

void CrashHandler::install(const QString& logDir) noexcept {
    if (s_installed) {
        return;
    }
    s_installed = true;

    const std::wstring nativeDir = logDir.toStdWString();
    wcsncpy_s(s_logDirW, MAX_PATH, nativeDir.c_str(), _TRUNCATE);

    SetUnhandledExceptionFilter(UnhandledCrashFilter);
    _set_invalid_parameter_handler(InvalidParameterHandler);
    _set_purecall_handler(PureCallHandler);
    std::set_terminate(TerminateHandler);
}

} // namespace infra::platform

#endif // _WIN32
