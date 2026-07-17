/**
 * @file ReleaseParser.h
 * @brief Release metadata parsing and version comparison — no Qt dependency.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace core::update {

struct ReleaseInfo {
    std::string tagName;
    std::string htmlUrl;
    bool draft = false;
    bool prerelease = false;
    std::string jsonBody; // raw JSON string for asset extraction
};

/**
 * @brief Release parser. Extracted from ui::common::UpdateChecker so that
 * version comparison and release filtering can be unit-tested in isolation.
 * parseReleases uses QJsonDocument (Qt6::Core) for RFC 8259-compliant parsing.
 */
namespace release_parser {

/**
 * @brief Strip leading 'v'/'V' from a version string.
 */
std::string normalizeVersion(const std::string& raw);

/**
 * @brief Compare two semantic versions. Returns >0 if a > b, <0 if a < b, 0 if equal.
 */
int compareVersions(const std::string& a, const std::string& b);

/**
 * @brief Parse a GitHub Releases JSON array and return all
 * non-draft releases that satisfy the prerelease filter.
 * @param json A JSON array string (as returned by the GitHub API).
 * @param includePrerelease Whether to include prerelease versions.
 * @return All matching releases, ordered as they appear in the JSON.
 */
std::vector<ReleaseInfo> parseReleases(const std::string& json,
                                       bool includePrerelease);

} // namespace release_parser

} // namespace core::update