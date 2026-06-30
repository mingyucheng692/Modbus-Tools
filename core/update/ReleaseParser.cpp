/**
 * @file ReleaseParser.cpp
 * @brief Implementation of ReleaseParser.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ReleaseParser.h"
#include <algorithm>
#include <charconv>
#include <cctype>
#include <sstream>

namespace core::update {

std::string ReleaseParser::normalizeVersion(const std::string& raw) {
    std::string cleaned = raw;
    // Trim leading/trailing whitespace
    auto start = cleaned.find_first_not_of(" \t\r\n");
    auto end = cleaned.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return {};
    }
    cleaned = cleaned.substr(start, end - start + 1);

    if (!cleaned.empty() && (cleaned[0] == 'v' || cleaned[0] == 'V')) {
        cleaned = cleaned.substr(1);
    }
    return cleaned;
}

int ReleaseParser::compareVersions(const std::string& a, const std::string& b) {
    auto split = [](const std::string& s) -> std::vector<int> {
        std::vector<int> parts;
        std::string normalized = normalizeVersion(s);
        std::stringstream ss(normalized);
        std::string token;
        while (std::getline(ss, token, '.')) {
            int value = 0;
            const auto* begin = token.data();
            const auto* end = begin + token.size();
            const auto result = std::from_chars(begin, end, value);
            if (result.ec == std::errc() && result.ptr == end) {
                parts.push_back(value);
            } else {
                parts.push_back(0);
            }
        }
        return parts;
    };

    auto leftParts = split(a);
    auto rightParts = split(b);
    size_t maxLen = std::max(leftParts.size(), rightParts.size());
    leftParts.resize(maxLen, 0);
    rightParts.resize(maxLen, 0);

    for (size_t i = 0; i < maxLen; ++i) {
        if (leftParts[i] > rightParts[i]) return 1;
        if (leftParts[i] < rightParts[i]) return -1;
    }
    return 0;
}

std::vector<ReleaseInfo> ReleaseParser::parseReleases(const std::string& json,
                                                     bool includePrerelease) {
    std::vector<ReleaseInfo> results;

    // Minimal JSON parsing for GitHub Releases API
    // We look for the first non-draft, non-prerelease (if filtered) entry
    auto findString = [](const std::string& src, const std::string& key) -> std::string {
        auto pos = src.find(key);
        if (pos == std::string::npos) return {};
        pos = src.find('"', pos + key.length());
        if (pos == std::string::npos) return {};
        auto end = src.find('"', pos + 1);
        if (end == std::string::npos) return {};
        return src.substr(pos + 1, end - pos - 1);
    };

    auto findBool = [](const std::string& src, const std::string& key) -> bool {
        auto pos = src.find(key);
        if (pos == std::string::npos) return false;
        pos = src.find(':', pos + key.length());
        if (pos == std::string::npos) return false;
        // skip whitespace
        while (pos + 1 < src.length() && std::isspace(static_cast<unsigned char>(src[pos + 1]))) {
            ++pos;
        }
        return pos + 1 < src.length() && src[pos + 1] == 't';
    };

    // Find array start
    auto arrayStart = json.find('[');
    if (arrayStart == std::string::npos) return results;

    size_t pos = arrayStart + 1;
    while (pos < json.length()) {
        // Find next '{' (object start)
        auto objStart = json.find('{', pos);
        if (objStart == std::string::npos) break;

        // Find matching '}' 
        int depth = 0;
        auto objEnd = objStart;
        for (size_t i = objStart; i < json.length(); ++i) {
            if (json[i] == '{') ++depth;
            else if (json[i] == '}') {
                --depth;
                if (depth == 0) {
                    objEnd = i;
                    break;
                }
            }
        }

        std::string objStr = json.substr(objStart, objEnd - objStart + 1);

        bool isDraft = findBool(objStr, "\"draft\"");
        bool isPrerelease = findBool(objStr, "\"prerelease\"");

        if (isDraft) {
            pos = objEnd + 1;
            continue;
        }
        if (!includePrerelease && isPrerelease) {
            pos = objEnd + 1;
            continue;
        }

        ReleaseInfo info;
        info.tagName = findString(objStr, "\"tag_name\"");
        info.htmlUrl = findString(objStr, "\"html_url\"");
        info.draft = isDraft;
        info.prerelease = isPrerelease;
        info.jsonBody = objStr;
        results.push_back(std::move(info));

        pos = objEnd + 1;
    }

    return results;
}

} // namespace core::update
