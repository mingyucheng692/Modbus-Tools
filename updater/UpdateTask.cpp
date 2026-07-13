/**
 * @file UpdateTask.cpp
 * @brief Platform-independent JSON parsing for update task configuration.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateTask.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <optional>
#include <regex>

namespace updater {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

/// Encodes a Unicode codepoint as UTF-8 and appends to str.
void appendCodepointAsUtf8(std::string& str, unsigned int cp) {
    if (cp <= 0x7F) {
        str.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        str.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        str.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        str.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        str.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        str.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        str.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string jsonUnescape(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        const char ch = input[i];
        if (ch != '\\' || i + 1 >= input.size()) {
            result.push_back(ch);
            continue;
        }
        const char next = input[++i];
        switch (next) {
        case '"':  result.push_back('"');  break;
        case '\\': result.push_back('\\'); break;
        case '/':  result.push_back('/');  break;
        case 'b':  result.push_back('\b'); break;
        case 'f':  result.push_back('\f'); break;
        case 'n':  result.push_back('\n'); break;
        case 'r':  result.push_back('\r'); break;
        case 't':  result.push_back('\t'); break;
        case 'u':
            if (i + 4 < input.size()) {
                const std::string hex = input.substr(i + 1, 4);
                char* end = nullptr;
                const unsigned int code = static_cast<unsigned int>(std::strtoul(hex.c_str(), &end, 16));
                if (end && *end == '\0') {
                    appendCodepointAsUtf8(result, code);
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

std::optional<std::string> parseJsonString(const std::string& json, const std::string& key) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\"", std::regex::ECMAScript);
    std::smatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    const std::string decoded = jsonUnescape(match[1].str());
    if (decoded.empty() && !match[1].str().empty()) {
        return std::nullopt;
    }
    return decoded;
}

std::optional<std::uint32_t> parseJsonUInt(const std::string& json, const std::string& key) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(\\d+)", std::regex::ECMAScript);
    std::smatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    errno = 0;
    const std::string valueText = match[1].str();
    char* end = nullptr;
    const unsigned long value = std::strtoul(valueText.c_str(), &end, 10);
    if (end == valueText.c_str() || (end && *end != '\0') || errno == ERANGE ||
        value > static_cast<unsigned long>(std::numeric_limits<std::uint32_t>::max())) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
}

std::optional<int> parseJsonInt(const std::string& json, const std::string& key) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+)", std::regex::ECMAScript);
    std::smatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    errno = 0;
    const std::string valueText = match[1].str();
    char* end = nullptr;
    const long value = std::strtol(valueText.c_str(), &end, 10);
    if (end == valueText.c_str() || (end && *end != '\0') || errno == ERANGE ||
        value < static_cast<long>(std::numeric_limits<int>::min()) ||
        value > static_cast<long>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }
    return static_cast<int>(value);
}

std::optional<bool> parseJsonBool(const std::string& json, const std::string& key) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)", std::regex::ECMAScript);
    std::smatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return match[1].str() == "true";
}

} // namespace

bool parseTaskConfig(const std::string& json, UpdateTask& config) {
    if (json.empty()) {
        return false;
    }

    const auto schemaVersion = parseJsonInt(json, "schemaVersion");
    const auto launcherPid = parseJsonUInt(json, "launcherPid");
    const auto targetExePath = parseJsonString(json, "targetExePath");
    const auto newExePath = parseJsonString(json, "newExePath");
    const auto backupExePath = parseJsonString(json, "backupExePath");
    const auto expectedVersion = parseJsonString(json, "expectedVersion");
    const auto expectedSha256 = parseJsonString(json, "expectedSha256");
    const auto restartAfterUpdate = parseJsonBool(json, "restartAfterUpdate");

    if (!schemaVersion || !launcherPid || !targetExePath || !newExePath ||
        !backupExePath || !expectedSha256 || !restartAfterUpdate) {
        return false;
    }

    config.schemaVersion = *schemaVersion;
    config.launcherPid = *launcherPid;
    config.targetExePath = trim(*targetExePath);
    config.newExePath = trim(*newExePath);
    config.backupExePath = trim(*backupExePath);
    config.expectedVersion = expectedVersion ? trim(*expectedVersion) : "";
    config.expectedSha256 = toLower(trim(*expectedSha256));
    config.restartAfterUpdate = *restartAfterUpdate;

    if (config.schemaVersion <= 0 || config.targetExePath.empty() ||
        config.newExePath.empty() || config.backupExePath.empty()) {
        return false;
    }
    if (config.expectedSha256.size() != 64) {
        return false;
    }
    for (char ch : config.expectedSha256) {
        if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))) {
            return false;
        }
    }
    return true;
}

} // namespace updater
