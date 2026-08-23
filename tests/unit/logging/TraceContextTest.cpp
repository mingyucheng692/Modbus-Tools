/**
 * @file TraceContextTest.cpp
 * @brief Unit tests for modbus::trace::Scope RAII semantics and the
 *        thread_local isolation of currentTraceId.
 */

#include <gtest/gtest.h>

#include "infra/logging/TraceContext.h"

#include <thread>

using modbus::trace::currentTraceId;
using modbus::trace::Scope;

class TraceContextTest : public ::testing::Test {
protected:
    void SetUp() override { currentTraceId = 0; }
    void TearDown() override { currentTraceId = 0; }
};

TEST_F(TraceContextTest, ScopePublishesIdAndRestoresZeroOnExit) {
    EXPECT_EQ(currentTraceId, 0u);
    {
        Scope scope(42);
        EXPECT_EQ(currentTraceId, 42u);
    }
    EXPECT_EQ(currentTraceId, 0u);
}

TEST_F(TraceContextTest, EarlyReturnStillRestores) {
    const auto runWithEarlyReturn = [](bool bailOut) -> quint64 {
        Scope scope(7);
        if (bailOut) {
            return currentTraceId;
        }
        return currentTraceId;
    };
    EXPECT_EQ(runWithEarlyReturn(true), 7u);
    EXPECT_EQ(currentTraceId, 0u);
    EXPECT_EQ(runWithEarlyReturn(false), 7u);
    EXPECT_EQ(currentTraceId, 0u);
}

TEST_F(TraceContextTest, NestedScopeRestoresOuterValue) {
    // Save/restore semantics: the inner scope must not erase the outer
    // trace context when it ends.
    Scope outer(100);
    EXPECT_EQ(currentTraceId, 100u);
    {
        Scope inner(200);
        EXPECT_EQ(currentTraceId, 200u);
    }
    EXPECT_EQ(currentTraceId, 100u);
}

TEST_F(TraceContextTest, ThreadLocalIsolation) {
    Scope scope(999);

    quint64 observedByOtherThread = 1;  // sentinel, must become 0
    std::thread other([&observedByOtherThread]() {
        observedByOtherThread = currentTraceId;
        // A Scope on the other thread stays on that thread.
        Scope otherScope(5);
        EXPECT_EQ(currentTraceId, 5u);
    });
    other.join();

    EXPECT_EQ(observedByOtherThread, 0u);   // no leak from the main thread
    EXPECT_EQ(currentTraceId, 999u);        // main thread unaffected
}
