// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace core::update {

// Role: Model holding GitHub release metadata (tag name, url, flags, raw body) for update discovery.
// Thread-safety: Value type; thread-safe for independent instances.
// Ownership: Value object passed by copy or move.
struct ReleaseInfo {
    std::string tagName;
    std::string htmlUrl;
    bool draft = false;
    bool prerelease = false;
    std::string jsonBody; // raw JSON string for asset extraction
};

// Pure-logic release parser and SemVer comparator extracted from UpdateChecker for unit testing.
namespace release_parser {

std::string normalizeVersion(const std::string& raw);

// Compares semantic versions with SemVer 2.0.0 prerelease precedence; build metadata ignored.
int compareVersions(const std::string& a, const std::string& b);

// Parses GitHub releases JSON array and returns non-draft releases matching prerelease filter.
std::vector<ReleaseInfo> parseReleases(const std::string& json,
                                       bool includePrerelease);

} // namespace release_parser

} // namespace core::update
