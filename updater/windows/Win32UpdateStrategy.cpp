/**
 * @file Win32UpdateStrategy.cpp
 * @brief Windows implementation of IPlatformUpdateStrategy using Win32 API.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "Win32UpdateStrategy.h"

#include <windows.h>
#include <bcrypt.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace updater {

namespace {

/// Converts UTF-8 string to UTF-16 wstring (Win32 boundary conversion).
std::wstring utf8ToWide(const std::string& utf8) {
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
std::string wideToUtf8(const std::wstring& wide) {
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

std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(bytes.size() * 2);
    for (unsigned char value : bytes) {
        hex.push_back(digits[(value >> 4) & 0x0F]);
        hex.push_back(digits[value & 0x0F]);
    }
    return hex;
}

} // namespace

std::string Win32UpdateStrategy::readAllBytes(const std::string& path) {
    const std::wstring wpath = utf8ToWide(path);
    std::ifstream file(std::filesystem::path(wpath), std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    return std::string(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());
}

bool Win32UpdateStrategy::computeSha256(const std::string& filePath, std::string& sha256) {
    const std::wstring wpath = utf8ToWide(filePath);

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hashHandle = nullptr;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    std::vector<unsigned char> hashObject;
    std::vector<unsigned char> hashValue;
    std::vector<unsigned char> buffer(1024 * 128);

    NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status < 0) {
        return false;
    }

    DWORD objectLength = 0;
    DWORD hashLength = 0;
    DWORD resultLength = 0;
    status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
                               reinterpret_cast<PUCHAR>(&objectLength),
                               sizeof(DWORD), &resultLength, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }
    status = BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH,
                               reinterpret_cast<PUCHAR>(&hashLength),
                               sizeof(DWORD), &resultLength, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    hashObject.resize(objectLength);
    hashValue.resize(hashLength);
    status = BCryptCreateHash(algorithm, &hashHandle, hashObject.data(),
                              objectLength, nullptr, 0, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    fileHandle = CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                             nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        BCryptDestroyHash(hashHandle);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    DWORD bytesRead = 0;
    while (true) {
        const BOOL readOk = ReadFile(fileHandle, buffer.data(),
                                     static_cast<DWORD>(buffer.size()), &bytesRead, nullptr);
        if (!readOk) {
            CloseHandle(fileHandle);
            BCryptDestroyHash(hashHandle);
            BCryptCloseAlgorithmProvider(algorithm, 0);
            return false;
        }
        if (bytesRead == 0) {
            break;
        }
        status = BCryptHashData(hashHandle, buffer.data(), bytesRead, 0);
        if (status < 0) {
            CloseHandle(fileHandle);
            BCryptDestroyHash(hashHandle);
            BCryptCloseAlgorithmProvider(algorithm, 0);
            return false;
        }
    }

    status = BCryptFinishHash(hashHandle, hashValue.data(), hashLength, 0);
    CloseHandle(fileHandle);
    BCryptDestroyHash(hashHandle);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (status < 0) {
        return false;
    }

    sha256 = bytesToHex(hashValue);
    return true;
}

bool Win32UpdateStrategy::fileExists(const std::string& path) {
    const std::wstring wpath = utf8ToWide(path);
    const DWORD attrs = GetFileAttributesW(wpath.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool Win32UpdateStrategy::moveFileAtomic(const std::string& source, const std::string& destination) {
    const std::wstring wsource = utf8ToWide(source);
    const std::wstring wdest = utf8ToWide(destination);
    return MoveFileExW(wsource.c_str(), wdest.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED |
                       MOVEFILE_WRITE_THROUGH) != FALSE;
}

bool Win32UpdateStrategy::launchTarget(const std::string& targetPath) {
    const std::wstring wpath = utf8ToWide(targetPath);
    STARTUPINFOW startupInfo{};
    PROCESS_INFORMATION processInfo{};
    startupInfo.cb = sizeof(startupInfo);
    std::wstring commandLine = L"\"" + wpath + L"\"";
    if (CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, FALSE,
                       0, nullptr, nullptr, &startupInfo, &processInfo)) {
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return true;
    }
    return false;
}

bool Win32UpdateStrategy::waitForLauncherExit(std::uint32_t pid) {
    if (pid == 0) {
        return true;
    }
    HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!processHandle) {
        return false;
    }
    const DWORD waitResult = WaitForSingleObject(processHandle, 30000);
    CloseHandle(processHandle);
    return waitResult == WAIT_OBJECT_0;
}

void Win32UpdateStrategy::showError(const std::string& message) {
    const std::wstring wmessage = utf8ToWide(message);
    MessageBoxW(nullptr, wmessage.c_str(), L"Update Error", MB_OK | MB_ICONERROR);
}

} // namespace updater
