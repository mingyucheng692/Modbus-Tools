/**
 * @file GenericChannelViewBase.cpp
 * @brief Implementation of GenericChannelViewBase.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericChannelViewBase.h"
#include "../common/ISettingsService.h"
#include "../widgets/BaseConnectionWidget.h"
#include "../widgets/ByteMonitorWidget.h"
#include "../widgets/GenericInputWidget.h"
#include "../widgets/CollapsibleSection.h"
#include "../common/ConnectionAlert.h"
#include "../../infra/io/ChannelOperationWorker.h"
#include <QThread>
#include <QEvent>
#include <QMetaObject>
#include <spdlog/spdlog.h>

namespace ui::views {

GenericChannelViewBase::GenericChannelViewBase(ui::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

GenericChannelViewBase::~GenericChannelViewBase() noexcept = default;

void GenericChannelViewBase::onDisconnectClicked() {
    if (!worker_) {
        return;
    }

    stopReconnectTimer();
    reconnectPolicy_.reset();
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericChannelViewBase::onSendRequested(const QByteArray& data) {
    if (!worker_) {
        return;
    }
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    QMetaObject::invokeMethod(worker_, "write", 
                              Qt::QueuedConnection, 
                              Q_ARG(QByteArray, data));
}

void GenericChannelViewBase::onFileSendRequested(const QString& filePath) {
    if (!worker_) {
        return;
    }
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }
    QMetaObject::invokeMethod(worker_, "sendFile",
                              Qt::QueuedConnection,
                              Q_ARG(QString, filePath),
                              Q_ARG(int, 4096)); // default chunk size
}

void GenericChannelViewBase::onWorkerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("Channel") : deviceHint;
    if (monitor_) {
        monitor_->appendError(tr("Error: %1").arg(error));
    }
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
}

void GenericChannelViewBase::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (monitor_) {
        monitor_->appendMessage(isTx, data);
    }
}

void GenericChannelViewBase::startWorker() {
    workerThread_ = new QThread();
    auto* worker = new io::ChannelOperationWorker();
    worker->moveToThread(workerThread_);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    setupCommonWorkerConnections(worker);

    workerThread_->start();
    worker_ = worker;
}

void GenericChannelViewBase::stopWorker() {
    stopReconnectTimer();

    auto* thread = workerThread_;
    auto* worker = worker_;

    if (!thread) {
        workerThread_ = nullptr;
        worker_ = nullptr;
        return;
    }

    if (!worker) {
        workerThread_ = nullptr;
        worker_ = nullptr;
        if (thread->isRunning()) {
            thread->quit();
        } else {
            delete thread;
        }
        return;
    }

    // Disconnect before nullifying members to prevent signal loss
    // during the stop sequence (worker may still emit signals while cleaning up).
    worker->disconnect(this);

    workerThread_ = nullptr;
    worker_ = nullptr;

    if (!thread->isRunning()) {
        delete worker;
        delete thread;
        return;
    }

    QMetaObject::invokeMethod(worker, [worker, thread]() {
        QObject::connect(worker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
        worker->close();
        worker->deleteLater();
    }, Qt::QueuedConnection);
}

void GenericChannelViewBase::setupCommonWorkerConnections(io::ChannelOperationWorker* worker) {
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred, this, &GenericChannelViewBase::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor, this, &GenericChannelViewBase::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::fileTransferStarted, this, [this](const QString& filePath, qint64 totalBytes) {
        lastLoggedFileTransferPct_ = -1;
        if (monitor_) {
            monitor_->appendInfo(tr("File transfer started: %1 (%2 bytes)").arg(filePath).arg(totalBytes));
        }
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferProgress, this, [this](const QString& filePath, qint64 sentBytes, qint64 totalBytes) {
        if (totalBytes <= 0) {
            return;
        }
        const int progress = static_cast<int>((sentBytes * 100) / totalBytes);
        if (progress == lastLoggedFileTransferPct_) {
            return;
        }
        lastLoggedFileTransferPct_ = progress;
        if (monitor_) {
            monitor_->appendInfo(tr("File transfer progress: %1 %2/%3")
                                            .arg(filePath)
                                            .arg(sentBytes)
                                            .arg(totalBytes));
        }
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFinished, this, [this](const QString& filePath) {
        if (monitor_) {
            monitor_->appendInfo(tr("File transfer finished: %1").arg(filePath));
        }
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferFailed, this, [this](const QString& filePath, const QString& error) {
        if (monitor_) {
            monitor_->appendInfo(tr("File transfer failed: %1 (%2)").arg(filePath, error));
        }
    });
    connect(worker, &io::ChannelOperationWorker::fileTransferCanceled, this, [this](const QString& filePath) {
        if (monitor_) {
            monitor_->appendInfo(tr("File transfer canceled: %1").arg(filePath));
        }
    });
}

void GenericChannelViewBase::startReconnectTimer(widgets::BaseConnectionWidget* connectionWidget) {
    if (!connectionWidget || !connectionWidget->autoReconnectEnabled()) {
        return;
    }

    if (reconnectPolicy_.exhausted()) {
        if (monitor_) {
            monitor_->appendInfo(tr("Auto-reconnect exhausted (%1 attempts)")
                                     .arg(reconnectPolicy_.maxRetries()));
        }
        stopReconnectTimer();
        return;
    }

    reconnectPolicy_.onFailed();
    const int delay = connectionWidget->reconnectDelayMs();
    reconnectPolicy_.setDelayMs(delay);

    if (monitor_) {
        monitor_->appendInfo(tr("Auto-reconnect in %1ms (%2)")
                                 .arg(delay)
                                 .arg(reconnectPolicy_.statusString()));
    }
    reconnectTimer_->start(delay);
}

void GenericChannelViewBase::stopReconnectTimer() {
    if (reconnectTimer_) {
        reconnectTimer_->stop();
    }
}

void GenericChannelViewBase::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views
