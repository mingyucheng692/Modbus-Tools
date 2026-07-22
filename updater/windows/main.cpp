/**
 * @file main.cpp
 * @brief Windows entry point for the OTA updater.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "../UpdateTask.h"
#include "Win32Encoding.h"
#include "Win32UpdateStrategy.h"

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <string>

namespace {

enum class Language {
    English,
    ZhCn,
    ZhTw
};

Language g_lang = Language::English;

std::string getString(const char* en, const char* zhCn, const char* zhTw) {
    switch (g_lang) {
    case Language::ZhCn: return zhCn;
    case Language::ZhTw: return zhTw;
    default: return en;
    }
}

std::wstring toLowerWide(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(towlower(ch));
    });
    return value;
}

std::string toLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

/// Parses CLI arguments. Supports both legacy --task mode and direct
/// parameter mode (--target-exe, --new-exe, etc.). Returns true if the
/// task was built from individual arguments (no file I/O needed).
struct CliParseResult {
    std::string taskPath;       // legacy --task path
    updater::UpdateTask task;   // built from direct args (valid when taskPath is empty)
    bool fromDirectArgs = false;
};

CliParseResult parseCommandLine() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return {};
    }

    CliParseResult result;
    std::wstring targetExe, newExe, backupExe, expectedSha256, expectedVersion, launcherPidStr;
    bool restartExplicitlySet = false;

    for (int i = 1; i < argc; ++i) {
        const std::wstring arg(argv[i]);
        if (arg == L"--task" && i + 1 < argc) {
            result.taskPath = updater::win32::wideToUtf8(argv[++i]);
        } else if (arg == L"--lang" && i + 1 < argc) {
            const std::wstring code = toLowerWide(argv[++i]);
            if (code == L"zh_cn") {
                g_lang = Language::ZhCn;
            } else if (code == L"zh_tw") {
                g_lang = Language::ZhTw;
            }
        } else if (arg == L"--target-exe" && i + 1 < argc) {
            targetExe = argv[++i];
        } else if (arg == L"--new-exe" && i + 1 < argc) {
            newExe = argv[++i];
        } else if (arg == L"--backup-exe" && i + 1 < argc) {
            backupExe = argv[++i];
        } else if (arg == L"--expected-sha256" && i + 1 < argc) {
            expectedSha256 = argv[++i];
        } else if (arg == L"--expected-version" && i + 1 < argc) {
            expectedVersion = argv[++i];
        } else if (arg == L"--launcher-pid" && i + 1 < argc) {
            launcherPidStr = argv[++i];
        } else if (arg == L"--restart") {
            result.task.restartAfterUpdate = true;
            restartExplicitlySet = true;
        } else if (arg == L"--no-restart") {
            result.task.restartAfterUpdate = false;
            restartExplicitlySet = true;
        }
    }
    LocalFree(argv);

    // If direct args were provided, build the task in-process (no file I/O).
    if (!targetExe.empty() && !newExe.empty() && !expectedSha256.empty()) {
        result.task.targetExePath = updater::win32::wideToUtf8(targetExe);
        result.task.newExePath = updater::win32::wideToUtf8(newExe);
        result.task.backupExePath = backupExe.empty()
            ? (result.task.targetExePath + ".bak")
            : updater::win32::wideToUtf8(backupExe);
        result.task.expectedSha256 = toLowerAscii(updater::win32::wideToUtf8(expectedSha256));
        result.task.expectedVersion = updater::win32::wideToUtf8(expectedVersion);
        result.task.schemaVersion = 1;
        if (!launcherPidStr.empty()) {
            result.task.launcherPid = static_cast<std::uint32_t>(
                std::wcstoul(launcherPidStr.c_str(), nullptr, 10));
        }
        if (!restartExplicitlySet) {
            result.task.restartAfterUpdate = true;
        }
        result.fromDirectArgs = true;
    }

    return result;
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    const CliParseResult cli = parseCommandLine();
    updater::UpdateTask task;

    updater::Win32UpdateStrategy strategy;

    if (cli.fromDirectArgs) {
        // Task built from CLI arguments — no file I/O, no TOCTOU surface.
        task = cli.task;
    } else {
        // Legacy --task path: read and parse task.json.
        if (cli.taskPath.empty()) {
            return 1;
        }
        const std::string json = strategy.readAllBytes(cli.taskPath);
        if (json.empty() || !updater::parseTaskConfig(json, task)) {
            strategy.showError(getString(
                "Failed to parse update task configuration.",
                "解析更新任务配置失败。",
                "解析更新任務設定失敗。"));
            return 1;
        }
    }

    // 1. Wait for launcher process to exit
    if (!strategy.waitForLauncherExit(task.launcherPid)) {
        strategy.showError(getString(
            "Timed out waiting for the main program to exit.",
            "等待主程序退出超时。",
            "等待主程式退出逾時。"));
        return 2;
    }

    // 2. Verify SHA-256 checksum
    std::string actualSha256;
    if (!strategy.computeSha256(task.newExePath, actualSha256) ||
        toLowerAscii(actualSha256) != task.expectedSha256) {
        strategy.showError(getString(
            "The downloaded update file is corrupted (checksum mismatch).",
            "下载的更新文件已损坏（校验码验证失败）。",
            "下載的更新檔案已損毀（校驗碼驗證失敗）。"));
        return 3;
    }

    // 3. Backup old version
    const bool hasOldTarget = strategy.fileExists(task.targetExePath);
    if (hasOldTarget) {
        if (!strategy.moveFileAtomic(task.targetExePath, task.backupExePath)) {
            strategy.showError(getString(
                "Failed to backup the old version. Please ensure the program is not being used by another process.",
                "备份旧版本失败。请确保程序未被其他进程占用。",
                "備份舊版本失敗。請確保程式未被其他進程佔用。"));
            return 4;
        }
    }

    // 4. Install new version (with rollback on failure)
    if (!strategy.moveFileAtomic(task.newExePath, task.targetExePath)) {
        if (hasOldTarget) {
            if (strategy.moveFileAtomic(task.backupExePath, task.targetExePath)) {
                if (task.restartAfterUpdate) {
                    strategy.launchTarget(task.targetExePath);
                }
                strategy.showError(getString(
                    "Failed to install the new version. The application has been restored to the old version.",
                    "安装新版本失败。应用程序已还原至旧版本。",
                    "安裝新版本失敗。應用程式已還原至舊版本。"));
                return 5;
            }
        }
        strategy.showError(getString(
            "Failed to install the new version and restoration failed. Please reinstall the application.",
            "安装新版本失败且无法还原。请重新安装应用程序。",
            "安裝新版本失敗且無法還原。請重新安裝應用程式。"));
        return 5;
    }

    // 5. Restart updated application
    if (task.restartAfterUpdate) {
        strategy.launchTarget(task.targetExePath);
    }
    return 0;
}
