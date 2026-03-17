#include <windows.h>
#include <shellapi.h>
#include <bcrypt.h>

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace {

struct TaskConfig {
    int schemaVersion = 0;
    std::uint32_t launcherPid = 0;
    std::wstring targetExePath;
    std::wstring newExePath;
    std::wstring backupExePath;
    std::wstring expectedVersion;
    std::wstring expectedSha256;
    bool restartAfterUpdate = true;
};

std::wstring toLower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(towlower(ch));
    });
    return value;
}

std::wstring trim(const std::wstring& value) {
    const auto begin = value.find_first_not_of(L" \t\r\n");
    if (begin == std::wstring::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(L" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::wstring readFileUtf8(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) {
        return {};
    }
    int required = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (required <= 0) {
        return {};
    }
    std::wstring text(required, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), text.data(), required);
    return text;
}

std::wstring jsonUnescape(const std::wstring& input) {
    std::wstring result;
    result.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        const wchar_t ch = input[i];
        if (ch != L'\\' || i + 1 >= input.size()) {
            result.push_back(ch);
            continue;
        }
        const wchar_t next = input[++i];
        switch (next) {
        case L'"': result.push_back(L'"'); break;
        case L'\\': result.push_back(L'\\'); break;
        case L'/': result.push_back(L'/'); break;
        case L'b': result.push_back(L'\b'); break;
        case L'f': result.push_back(L'\f'); break;
        case L'n': result.push_back(L'\n'); break;
        case L'r': result.push_back(L'\r'); break;
        case L't': result.push_back(L'\t'); break;
        case L'u':
            if (i + 4 < input.size()) {
                const std::wstring hex = input.substr(i + 1, 4);
                wchar_t* end = nullptr;
                const auto code = static_cast<wchar_t>(wcstol(hex.c_str(), &end, 16));
                if (end && *end == L'\0') {
                    result.push_back(code);
                    i += 4;
                    break;
                }
            }
            return {};
        default:
            return {};
        }
    }
    return result;
}

std::optional<std::wstring> parseJsonString(const std::wstring& json, const std::wstring& key) {
    const std::wregex pattern(L"\"" + key + L"\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\"", std::regex::ECMAScript);
    std::wsmatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    const std::wstring decoded = jsonUnescape(match[1].str());
    if (decoded.empty() && !match[1].str().empty()) {
        return std::nullopt;
    }
    return decoded;
}

std::optional<std::uint32_t> parseJsonUInt(const std::wstring& json, const std::wstring& key) {
    const std::wregex pattern(L"\"" + key + L"\"\\s*:\\s*(\\d+)", std::regex::ECMAScript);
    std::wsmatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    try {
        return static_cast<std::uint32_t>(std::stoul(match[1].str()));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int> parseJsonInt(const std::wstring& json, const std::wstring& key) {
    const std::wregex pattern(L"\"" + key + L"\"\\s*:\\s*(-?\\d+)", std::regex::ECMAScript);
    std::wsmatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    try {
        return std::stoi(match[1].str());
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> parseJsonBool(const std::wstring& json, const std::wstring& key) {
    const std::wregex pattern(L"\"" + key + L"\"\\s*:\\s*(true|false)", std::regex::ECMAScript);
    std::wsmatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return match[1].str() == L"true";
}

bool parseTaskConfig(const std::wstring& taskPath, TaskConfig& config) {
    const std::wstring json = readFileUtf8(taskPath);
    if (json.empty()) {
        return false;
    }

    const auto schemaVersion = parseJsonInt(json, L"schemaVersion");
    const auto launcherPid = parseJsonUInt(json, L"launcherPid");
    const auto targetExePath = parseJsonString(json, L"targetExePath");
    const auto newExePath = parseJsonString(json, L"newExePath");
    const auto backupExePath = parseJsonString(json, L"backupExePath");
    const auto expectedVersion = parseJsonString(json, L"expectedVersion");
    const auto expectedSha256 = parseJsonString(json, L"expectedSha256");
    const auto restartAfterUpdate = parseJsonBool(json, L"restartAfterUpdate");

    if (!schemaVersion || !launcherPid || !targetExePath || !newExePath || !backupExePath || !expectedSha256 || !restartAfterUpdate) {
        return false;
    }

    config.schemaVersion = *schemaVersion;
    config.launcherPid = *launcherPid;
    config.targetExePath = trim(*targetExePath);
    config.newExePath = trim(*newExePath);
    config.backupExePath = trim(*backupExePath);
    config.expectedVersion = expectedVersion ? trim(*expectedVersion) : L"";
    config.expectedSha256 = toLower(trim(*expectedSha256));
    config.restartAfterUpdate = *restartAfterUpdate;

    if (config.schemaVersion <= 0 || config.targetExePath.empty() || config.newExePath.empty() || config.backupExePath.empty()) {
        return false;
    }
    if (config.expectedSha256.size() != 64) {
        return false;
    }
    for (wchar_t ch : config.expectedSha256) {
        if (!((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f'))) {
            return false;
        }
    }
    return true;
}

bool waitForLauncherExit(std::uint32_t pid) {
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

std::wstring bytesToHex(const std::vector<unsigned char>& bytes) {
    static constexpr wchar_t digits[] = L"0123456789abcdef";
    std::wstring hex;
    hex.reserve(bytes.size() * 2);
    for (unsigned char value : bytes) {
        hex.push_back(digits[(value >> 4) & 0x0F]);
        hex.push_back(digits[value & 0x0F]);
    }
    return hex;
}

bool computeSha256(const std::wstring& filePath, std::wstring& sha256) {
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
    status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectLength), sizeof(DWORD), &resultLength, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }
    status = BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashLength), sizeof(DWORD), &resultLength, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    hashObject.resize(objectLength);
    hashValue.resize(hashLength);
    status = BCryptCreateHash(algorithm, &hashHandle, hashObject.data(), objectLength, nullptr, 0, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        BCryptDestroyHash(hashHandle);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return false;
    }

    DWORD bytesRead = 0;
    while (true) {
        const BOOL readOk = ReadFile(fileHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr);
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

bool fileExists(const std::wstring& path) {
    const DWORD attrs = GetFileAttributesW(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool moveFileAtomic(const std::wstring& source, const std::wstring& destination) {
    return MoveFileExW(source.c_str(),
                       destination.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) != FALSE;
}

void launchTarget(const std::wstring& targetPath) {
    STARTUPINFOW startupInfo{};
    PROCESS_INFORMATION processInfo{};
    startupInfo.cb = sizeof(startupInfo);
    std::wstring commandLine = L"\"" + targetPath + L"\"";
    if (CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo)) {
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
    }
}

std::wstring getTaskPathFromArgs() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv || argc < 3) {
        if (argv) {
            LocalFree(argv);
        }
        return {};
    }

    std::wstring taskPath;
    for (int i = 1; i < argc - 1; ++i) {
        if (std::wstring(argv[i]) == L"--task") {
            taskPath = argv[i + 1];
            break;
        }
    }
    LocalFree(argv);
    return taskPath;
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    const std::wstring taskPath = getTaskPathFromArgs();
    if (taskPath.empty()) {
        return 1;
    }

    TaskConfig task;
    if (!parseTaskConfig(taskPath, task)) {
        return 1;
    }

    if (!waitForLauncherExit(task.launcherPid)) {
        return 2;
    }

    std::wstring actualSha256;
    if (!computeSha256(task.newExePath, actualSha256) || toLower(actualSha256) != task.expectedSha256) {
        return 3;
    }

    const bool hasOldTarget = fileExists(task.targetExePath);
    if (hasOldTarget) {
        DeleteFileW(task.backupExePath.c_str());
        if (!moveFileAtomic(task.targetExePath, task.backupExePath)) {
            return 4;
        }
    }

    if (!moveFileAtomic(task.newExePath, task.targetExePath)) {
        if (hasOldTarget) {
            if (moveFileAtomic(task.backupExePath, task.targetExePath)) {
                if (task.restartAfterUpdate) {
                    launchTarget(task.targetExePath);
                }
                return 5;
            }
            return 6;
        }
        return 5;
    }

    if (task.restartAfterUpdate) {
        launchTarget(task.targetExePath);
    }
    return 0;
}
