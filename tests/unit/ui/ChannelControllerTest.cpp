// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "ui/views/ChannelController.h"
#include "infra/io/ChannelOperationWorker.h"

#include <QCoreApplication>

namespace {

TEST(ChannelControllerTest, CreateAndDestroyWorkerRapidlyWithoutLeakOrAbort) {
    QObject receiver;
    ui::views::ChannelController controller;

    for (int i = 0; i < 5; ++i) {
        auto* worker = controller.createWorker();
        ASSERT_NE(worker, nullptr);
        EXPECT_TRUE(controller.hasWorker());
        EXPECT_EQ(controller.worker(), worker);

        controller.destroyWorker(&receiver);
        EXPECT_FALSE(controller.hasWorker());
        EXPECT_EQ(controller.worker(), nullptr);
    }

    QCoreApplication::processEvents();
}

TEST(ChannelControllerTest, DestructorReleasesActiveWorkerCleanly) {
    QObject receiver;
    {
        ui::views::ChannelController controller;
        auto* worker = controller.createWorker();
        ASSERT_NE(worker, nullptr);
        QObject::connect(worker, &io::ChannelOperationWorker::stateChanged,
                         &receiver, [](io::ChannelState) {});
    }

    QCoreApplication::processEvents();
    SUCCEED();
}

} // namespace
