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
        return worker_;
    }

    workerThread_ = new QThread();
    auto* worker = new io::ChannelOperationWorker();
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    workerThread_->start();
    worker_ = worker;
    return worker_;
}

void ChannelController::destroyWorker() {
    stopAutoSend();
    auto* thread = workerThread_;
    auto* worker = worker_;
    workerThread_ = nullptr;
    worker_ = nullptr;
    stopWorkerPair(thread, worker,
                   {},
                   [](QObject* w) {
                       static_cast<io::ChannelOperationWorker*>(w)->close();
                   });
}

void ChannelController::write(const QByteArray& data) {
    if (!worker_) {
        return;
    }
    QMetaObject::invokeMethod(worker_, "write",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void ChannelController::disconnect() {
    stopAutoSend();
    if (!worker_) {
        return;
    }
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
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

void ChannelController::stopWorkerPair(QThread* thread, QObject* worker,
                                       const std::function<void()>& onPreStop,
                                       const std::function<void(QObject*)>& onCloseInThread) {
    if (onPreStop) {
        onPreStop();
    }

    if (!thread) {
        return;
    }

    if (!worker) {
        if (thread->isRunning()) {
            thread->quit();
        } else {
            delete thread;
        }
        return;
    }

    // Disconnect before shutdown to prevent signal loss
    // during the stop sequence (worker may still emit signals while cleaning up).
    worker->disconnect(this);

    if (!thread->isRunning()) {
        delete worker;
        delete thread;
        return;
    }

    QMetaObject::invokeMethod(worker, [worker, thread, onCloseInThread]() {
        QObject::connect(worker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
        if (onCloseInThread) {
            onCloseInThread(worker);
        }
        worker->deleteLater();
    }, Qt::QueuedConnection);
}

} // namespace ui::views
