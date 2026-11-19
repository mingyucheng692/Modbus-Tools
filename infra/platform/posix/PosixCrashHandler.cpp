// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "infra/platform/CrashHandler.h"

#if !defined(_WIN32)

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#if defined(__GLIBC__) || defined(__APPLE__)
#include <execinfo.h>
#endif

namespace infra::platform {

namespace {

char s_logDirA[4096] = {0};
bool s_installed = false;

void posixSignalHandler(int sig, siginfo_t* info, void* /*ucontext*/) noexcept {
    // Re-arm default handler immediately to prevent infinite signal loops
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(sig, &sa, nullptr);

    char filePath[4096];
    time_t now = time(nullptr);
    struct tm tmNow;
    localtime_r(&now, &tmNow);

    char timeBuf[32];
    std::snprintf(timeBuf, sizeof(timeBuf), "%04d%02d%02d-%02d%02d%02d",
                  tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday,
                  tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

    std::snprintf(filePath, sizeof(filePath), "%s/crash_%s_%d.txt",
                  s_logDirA, timeBuf, static_cast<int>(getpid()));

    int fd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        char msg[512];
        int len = std::snprintf(msg, sizeof(msg),
            "=== POSIX CRASH REPORT ===\n"
            "Signal: %d\n"
            "Fault Address: %p\n"
            "Process ID: %d\n",
            sig,
            info ? info->si_addr : nullptr,
            static_cast<int>(getpid()));
        if (len > 0) {
            ssize_t ignored = write(fd, msg, static_cast<size_t>(len));
            (void)ignored;
        }

#if defined(__GLIBC__) || defined(__APPLE__)
        void* callstack[64];
        int frames = backtrace(callstack, 64);
        backtrace_symbols_fd(callstack, frames, fd);
#endif
        close(fd);
    }

    raise(sig);
}

} // namespace

void CrashHandler::install(const QString& logDir) noexcept {
    if (s_installed) {
        return;
    }
    s_installed = true;

    const std::string dirStr = logDir.toStdString();
    std::strncpy(s_logDirA, dirStr.c_str(), sizeof(s_logDirA) - 1);
    s_logDirA[sizeof(s_logDirA) - 1] = '\0';

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = posixSignalHandler;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
}

} // namespace infra::platform

#endif // !_WIN32
