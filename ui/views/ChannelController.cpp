// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ChannelController.h"
#include "../../infra/io/ChannelOperationWorker.h"
#include <QThread>
#include <QMetaObject>
#include <QTimer>
#include <algorithm>

namespace ui::views {

ChannelController::ChannelController(QObject* parent)
    : QObject(parent) {}

ChannelController::~ChannelController() noexcept {
    destroyWorker();
}

io::ChannelOperationWorker* ChannelController::createWorker() {
    if (worker_) {
        return worker_.get();
    }

    workerThread_ = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* rawWorker = new io::ChannelOperationWorker();
    rawWorker->moveToThread(workerThread_.get());
    worker_ = std::shared_ptr<io::ChannelOperationWorker>(
        rawWorker,
        [](io::ChannelOperationWorker* w) {
            core::common::ThreadGuard::releaseWorker(w, [w]() { w->close(); });
        });

    workerThread_->start();
    return worker_.get();
}

void ChannelController::destroyWorker(QObject* receiverToDisconnect) {
    stopAutoSend();
    releaseWorkerPair(workerThread_,
                      worker_,
                      receiverToDisconnect ? receiverToDisconnect : parent());
}

void ChannelController::write(const QByteArray& data) {
    if (!worker_) {
        return;
    }
    QMetaObject::invokeMethod(worker_.get(), "write",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void ChannelController::disconnect() {
    stopAutoSend();
    if (!worker_) {
        return;
    }
    QMetaObject::invokeMethod(worker_.get(), "close", Qt::QueuedConnection);
}

void ChannelController::startAutoSend(int intervalMs, std::function<void()> sendAction) {
    autoSendAction_ = std::move(sendAction);
    if (!autoSendTimer_) {
        autoSendTimer_ = new QTimer(this);
        connect(autoSendTimer_, &QTimer::timeout, this, [this]() {
            if (autoSendAction_) {
                autoSendAction_();
            }
        });
    }
    autoSendTimer_->start(std::max(1, intervalMs));
}

void ChannelController::stopAutoSend() {
    if (autoSendTimer_) {
        autoSendTimer_->stop();
    }
    autoSendAction_ = nullptr;
}

void ChannelController::setAutoSendInterval(int intervalMs) {
    if (autoSendTimer_ && autoSendTimer_->isActive()) {
        autoSendTimer_->setInterval(std::max(1, intervalMs));
    }
}

bool ChannelController::isAutoSendActive() const {
    return autoSendTimer_ && autoSendTimer_->isActive();
}

} // namespace ui::views
