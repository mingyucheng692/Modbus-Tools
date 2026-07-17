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

/// Parses --task and --lang command-line arguments. Returns UTF-8 task path.
std::string getTaskPathAndLang() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return {};
    }

    std::wstring taskPath;
    for (int i = 1; i < argc; ++i) {
        if (std::wstring(argv[i]) == L"--task" && i + 1 < argc) {
            taskPath = argv[i + 1];
        } else if (std::wstring(argv[i]) == L"--lang" && i + 1 < argc) {
            const std::wstring code = toLowerWide(argv[i + 1]);
            if (code == L"zh_cn") {
                g_lang = Language::ZhCn;
            } else if (code == L"zh_tw") {
                g_lang = Language::ZhTw;
            }
        }
    }
    LocalFree(argv);
    return updater::win32::wideToUtf8(taskPath);
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    const std::string taskPath = getTaskPathAndLang();
    if (taskPath.empty()) {
        return 1;
    }

    updater::Win32UpdateStrategy strategy;

    // 1. Read and parse task configuration
    const std::string json = strategy.readAllBytes(taskPath);
    updater::UpdateTask task;
    if (json.empty() || !updater::parseTaskConfig(json, task)) {
        strategy.showError(getString(
            "Failed to parse update task configuration.",
            "解析更新任务配置失败。",
            "解析更新任務設定失敗。"));
        return 1;
    }

    // 2. Wait for launcher process to exit
    if (!strategy.waitForLauncherExit(task.launcherPid)) {
        strategy.showError(getString(
            "Timed out waiting for the main program to exit.",
            "等待主程序退出超时。",
            "等待主程式退出逾時。"));
        return 2;
    }

    // 3. Verify SHA-256 checksum
    std::string actualSha256;
    if (!strategy.computeSha256(task.newExePath, actualSha256) ||
        toLowerAscii(actualSha256) != task.expectedSha256) {
        strategy.showError(getString(
            "The downloaded update file is corrupted (checksum mismatch).",
            "下载的更新文件已损坏（校验码验证失败）。",
            "下載的更新檔案已損毀（校驗碼驗證失敗）。"));
        return 3;
    }

    // 4. Backup old version
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

    // 5. Install new version (with rollback on failure)
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

    // 6. Restart updated application
    if (task.restartAfterUpdate) {
        strategy.launchTarget(task.targetExePath);
    }
    return 0;
}
