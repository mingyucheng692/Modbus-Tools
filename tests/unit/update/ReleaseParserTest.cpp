/**
 * @file ReleaseParserTest.cpp
 * @brief Unit tests for ReleaseParser.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "update/ReleaseParser.h"

using namespace core::update;

TEST(ReleaseParser, NormalizeVersionStripsV) {
    EXPECT_EQ(ReleaseParser::normalizeVersion("v1.2.3"), "1.2.3");
    EXPECT_EQ(ReleaseParser::normalizeVersion("V1.0.0"), "1.0.0");
    EXPECT_EQ(ReleaseParser::normalizeVersion("1.0.0"), "1.0.0");
}

TEST(ReleaseParser, NormalizeVersionTrimsWhitespace) {
    EXPECT_EQ(ReleaseParser::normalizeVersion("  v1.2.3  "), "1.2.3");
    EXPECT_EQ(ReleaseParser::normalizeVersion("\t1.0.0\n"), "1.0.0");
}

TEST(ReleaseParser, NormalizeVersionEmpty) {
    EXPECT_EQ(ReleaseParser::normalizeVersion(""), "");
    EXPECT_EQ(ReleaseParser::normalizeVersion("  "), "");
}

TEST(ReleaseParser, CompareVersionsEqual) {
    EXPECT_EQ(ReleaseParser::compareVersions("1.0.0", "1.0.0"), 0);
    EXPECT_EQ(ReleaseParser::compareVersions("v1.0.0", "1.0.0"), 0);
    EXPECT_EQ(ReleaseParser::compareVersions("2.0", "2.0.0"), 0);
}

TEST(ReleaseParser, CompareVersionsGreater) {
    EXPECT_GT(ReleaseParser::compareVersions("2.0.0", "1.0.0"), 0);
    EXPECT_GT(ReleaseParser::compareVersions("1.2.0", "1.1.0"), 0);
    EXPECT_GT(ReleaseParser::compareVersions("1.0.1", "1.0.0"), 0);
    EXPECT_GT(ReleaseParser::compareVersions("v2.0.0", "v1.9.9"), 0);
}

TEST(ReleaseParser, CompareVersionsLess) {
    EXPECT_LT(ReleaseParser::compareVersions("1.0.0", "2.0.0"), 0);
    EXPECT_LT(ReleaseParser::compareVersions("1.1.0", "1.2.0"), 0);
    EXPECT_LT(ReleaseParser::compareVersions("1.0.0", "1.0.1"), 0);
}

TEST(ReleaseParser, CompareVersionsDifferentLengths) {
    EXPECT_GT(ReleaseParser::compareVersions("1.0.0.1", "1.0.0"), 0);
    EXPECT_EQ(ReleaseParser::compareVersions("1.0", "1.0.0"), 0);
}

TEST(ReleaseParser, ParseReleasesEmpty) {
    auto results = ReleaseParser::parseReleases("", false);
    EXPECT_TRUE(results.empty());
}

TEST(ReleaseParser, ParseReleasesInvalidJson) {
    auto results = ReleaseParser::parseReleases("not json", false);
    EXPECT_TRUE(results.empty());
}

TEST(ReleaseParser, ParseReleasesSingleRelease) {
    const char* json = R"([
        {
            "tag_name": "v1.0.0",
            "html_url": "https://github.com/test/releases/tag/v1.0.0",
            "draft": false,
            "prerelease": false,
            "assets": []
        }
    ])";

    auto results = ReleaseParser::parseReleases(json, false);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].tagName, "v1.0.0");
    EXPECT_EQ(results[0].htmlUrl, "https://github.com/test/releases/tag/v1.0.0");
    EXPECT_FALSE(results[0].draft);
    EXPECT_FALSE(results[0].prerelease);
}

TEST(ReleaseParser, ParseReleasesFiltersDraft) {
    const char* json = R"([
        {
            "tag_name": "v1.0.0",
            "html_url": "https://github.com/test/releases/tag/v1.0.0",
            "draft": true,
            "prerelease": false,
            "assets": []
        },
        {
            "tag_name": "v2.0.0",
            "html_url": "https://github.com/test/releases/tag/v2.0.0",
            "draft": false,
            "prerelease": false,
            "assets": []
        }
    ])";

    auto results = ReleaseParser::parseReleases(json, false);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].tagName, "v2.0.0");
}

TEST(ReleaseParser, ParseReleasesFiltersPrerelease) {
    const char* json = R"([
        {
            "tag_name": "v1.0.0-beta",
            "html_url": "https://github.com/test/releases/tag/v1.0.0-beta",
            "draft": false,
            "prerelease": true,
            "assets": []
        },
        {
            "tag_name": "v1.0.0",
            "html_url": "https://github.com/test/releases/tag/v1.0.0",
            "draft": false,
            "prerelease": false,
            "assets": []
        }
    ])";

    auto results = ReleaseParser::parseReleases(json, false);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].tagName, "v1.0.0");
}

TEST(ReleaseParser, ParseReleasesIncludesPrerelease) {
    const char* json = R"([
        {
            "tag_name": "v1.0.0-beta",
            "html_url": "https://github.com/test/releases/tag/v1.0.0-beta",
            "draft": false,
            "prerelease": true,
            "assets": []
        },
        {
            "tag_name": "v1.0.0",
            "html_url": "https://github.com/test/releases/tag/v1.0.0",
            "draft": false,
            "prerelease": false,
            "assets": []
        }
    ])";

    auto results = ReleaseParser::parseReleases(json, true);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].tagName, "v1.0.0-beta");
    EXPECT_EQ(results[1].tagName, "v1.0.0");
}

TEST(ReleaseParser, ParseReleasesPreservesJsonBody) {
    const char* json = R"([
        {
            "tag_name": "v1.0.0",
            "html_url": "https://github.com/test/releases/tag/v1.0.0",
            "draft": false,
            "prerelease": false,
            "assets": [{"name": "test.zip"}]
        }
    ])";

    auto results = ReleaseParser::parseReleases(json, false);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].jsonBody.empty());
    EXPECT_NE(results[0].jsonBody.find("\"assets\""), std::string::npos);
}