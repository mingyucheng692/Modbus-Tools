/**
 * @file ChannelController.cpp
 * @brief Implementation of ChannelController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ChannelController.h"
#include "../../infra/io/ChannelOperationWorker.h"
#include <QThread>
#include <QMetaObject>

namespace ui::views {

ChannelController::ChannelController(QObject* parent)
    : QObject(parent) {
    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &ChannelController::reconnectTimeout);
}

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
    auto* thread = workerThread_;
    auto* worker = worker_;
    workerThread_ = nullptr;
    worker_ = nullptr;
    stopWorkerPair(thread, worker,
                   [this] { stopReconnectTimer(); },
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
    if (!worker_) {
        return;
    }
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void ChannelController::startReconnectTimer(int delayMs) {
    reconnectPolicy_.onFailed();
    reconnectPolicy_.setDelayMs(delayMs);
    reconnectTimer_->start(delayMs);
}

void ChannelController::stopReconnectTimer() {
    if (reconnectTimer_) {
        reconnectTimer_->stop();
    }
}

void ChannelController::resetReconnect() {
    reconnectPolicy_.reset();
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