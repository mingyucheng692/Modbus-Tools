// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "infra/platform/CrashHandler.h"
#include <QTemporaryDir>

TEST(CrashHandlerTest, InstallIsIdempotentAndSafe)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    EXPECT_NO_FATAL_FAILURE(infra::platform::CrashHandler::install(dir.path()));
    // Second call is safe and idempotent
    EXPECT_NO_FATAL_FAILURE(infra::platform::CrashHandler::install(dir.path()));
}
