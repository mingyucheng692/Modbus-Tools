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
#include <sstream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QByteArray>

namespace core::update {

std::string release_parser::normalizeVersion(const std::string& raw) {
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

int release_parser::compareVersions(const std::string& a, const std::string& b) {
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

std::vector<ReleaseInfo> release_parser::parseReleases(const std::string& json,
                                                     bool includePrerelease) {
    std::vector<ReleaseInfo> results;

    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    if (!doc.isArray()) return results;

    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) continue;
        const QJsonObject obj = value.toObject();
        const bool isDraft = obj.value("draft").toBool();
        const bool isPrerelease = obj.value("prerelease").toBool();
        if (isDraft) continue;
        if (!includePrerelease && isPrerelease) continue;

        ReleaseInfo info;
        info.tagName = obj.value("tag_name").toString().toUtf8().toStdString();
        info.htmlUrl = obj.value("html_url").toString().toUtf8().toStdString();
        info.draft = isDraft;
        info.prerelease = isPrerelease;
        info.jsonBody = QJsonDocument(obj).toJson(QJsonDocument::Compact).toStdString();
        results.push_back(std::move(info));
    }

    return results;
}

} // namespace core::update
