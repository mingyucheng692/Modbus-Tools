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
#include <cctype>
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

namespace {

/// Numeric + prerelease decomposition of a normalized version string.
/// "1.2.3-rc.1+build.5" → segments {1, 2, 3}, prerelease "rc.1"
/// (build metadata after '+' is ignored for precedence, per SemVer 2.0.0 §10).
struct VersionParts {
    std::vector<int> segments;
    std::string prerelease; // empty for a plain release
};

VersionParts parseVersionParts(const std::string& normalized) {
    VersionParts parts;

    std::string main = normalized;
    if (const auto plus = main.find('+'); plus != std::string::npos) {
        main = main.substr(0, plus);
    }
    if (const auto dash = main.find('-'); dash != std::string::npos) {
        parts.prerelease = main.substr(dash + 1);
        main = main.substr(0, dash);
    }

    std::stringstream ss(main);
    std::string token;
    while (std::getline(ss, token, '.')) {
        int value = 0;
        const auto* begin = token.data();
        const auto* end = begin + token.size();
        const auto result = std::from_chars(begin, end, value);
        // '-' was already split off above, so suffix tokens like "8-rc1" can
        // no longer swallow the numeric prefix ("8" used to be dropped and
        // pushed as 0, silently regressing the whole segment). Keep the
        // tolerance for malformed numeric tokens ("1.0.x" → 0).
        parts.segments.push_back(result.ec == std::errc() && result.ptr == end ? value : 0);
    }
    return parts;
}

bool isAllDigits(const std::string& id) {
    return !id.empty() && std::all_of(id.begin(), id.end(), [](unsigned char c) {
               return std::isdigit(c) != 0;
           });
}

int numericValue(const std::string& id) {
    int value = 0;
    std::from_chars(id.data(), id.data() + id.size(), value);
    return value;
}

/// SemVer 2.0.0 §11.4 prerelease identifier precedence, extended for this
/// project's "rcN" tag convention:
///  - numeric identifiers compare numerically and rank below alphanumeric ones;
///  - alphanumeric identifiers compare lexically (ASCII);
///  - identifier "label+digits" hybrids ("rc1", "beta2") order their numeric
///    tails numerically (rc2 < rc10) — pure ASCII comparison would invert
///    two-digit iterations (rc10 < rc2) and wrongly re-prompt rc users;
///  - with all preceding identifiers equal, the longer list outranks the shorter.
int comparePrereleaseIdentifiers(const std::string& lhs, const std::string& rhs) {
    // Returns true and fills label/tail for identifiers shaped ^[a-zA-Z]+[0-9]+$.
    auto splitHybrid = [](const std::string& id, std::string& label, int& tail) {
        const auto digitsStart = std::find_if_not(id.begin(), id.end(), [](unsigned char c) {
            return std::isalpha(c) != 0;
        });
        if (digitsStart == id.begin() || digitsStart == id.end()) {
            return false; // no label or no digits
        }
        if (!std::all_of(digitsStart, id.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) {
            return false;
        }
        label.assign(id.begin(), digitsStart);
        tail = numericValue(id.substr(digitsStart - id.begin()));
        return true;
    };

    size_t lpos = 0;
    size_t rpos = 0;
    bool lDone = false;
    bool rDone = false;
    while (!lDone || !rDone) {
        if (lDone || rDone) {
            return lDone ? -1 : 1;
        }
        const auto lEnd = lhs.find('.', lpos);
        const auto rEnd = rhs.find('.', rpos);
        const std::string lId = lhs.substr(lpos, lEnd == std::string::npos ? std::string::npos : lEnd - lpos);
        const std::string rId = rhs.substr(rpos, rEnd == std::string::npos ? std::string::npos : rEnd - rpos);
        lDone = (lEnd == std::string::npos);
        rDone = (rEnd == std::string::npos);
        lpos = lDone ? lhs.size() : lEnd + 1;
        rpos = rDone ? rhs.size() : rEnd + 1;

        std::string lLabel;
        std::string rLabel;
        int lTail = 0;
        int rTail = 0;
        const bool lHybrid = splitHybrid(lId, lLabel, lTail);
        const bool rHybrid = splitHybrid(rId, rLabel, rTail);
        if (lHybrid && rHybrid) {
            if (lLabel != rLabel) {
                return lLabel < rLabel ? -1 : 1;
            }
            if (lTail != rTail) {
                return lTail < rTail ? -1 : 1;
            }
            continue;
        }

        const bool lNumeric = isAllDigits(lId);
        const bool rNumeric = isAllDigits(rId);
        if (lNumeric && rNumeric) {
            const int lValue = numericValue(lId);
            const int rValue = numericValue(rId);
            if (lValue != rValue) {
                return lValue < rValue ? -1 : 1;
            }
        } else if (lNumeric != rNumeric) {
            return lNumeric ? -1 : 1;
        } else if (lId != rId) {
            return lId < rId ? -1 : 1;
        }
    }
    return 0;
}

} // namespace

int release_parser::compareVersions(const std::string& a, const std::string& b) {
    const VersionParts lhs = parseVersionParts(normalizeVersion(a));
    const VersionParts rhs = parseVersionParts(normalizeVersion(b));

    const size_t maxLen = std::max(lhs.segments.size(), rhs.segments.size());
    for (size_t i = 0; i < maxLen; ++i) {
        const int left = i < lhs.segments.size() ? lhs.segments[i] : 0;
        const int right = i < rhs.segments.size() ? rhs.segments[i] : 0;
        if (left != right) {
            return left < right ? -1 : 1;
        }
    }

    // Semantic decision (2026-09-13, R1): numeric segments decide first; for
    // equal segments a prerelease ranks below the plain release
    // ("1.0.8-rc1" < "1.0.8"), so prerelease channels stop reporting phantom
    // updates once the matching release lands, and rc iterations stay
    // distinguishable ("1.0.7-rc1" < "1.0.7-rc2"). Prerelease identifier
    // precedence follows SemVer 2.0.0 §11.4.
    if (lhs.prerelease == rhs.prerelease) {
        return 0;
    }
    if (lhs.prerelease.empty()) {
        return 1; // release > prerelease
    }
    if (rhs.prerelease.empty()) {
        return -1; // prerelease < release
    }
    return comparePrereleaseIdentifiers(lhs.prerelease, rhs.prerelease);
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
