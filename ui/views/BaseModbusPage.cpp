/**
 * @file BaseModbusPage.cpp
 * @brief Implementation of BaseModbusPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "BaseModbusPage.h"
#include "AppConstants.h"
#include "../common/ISettingsService.h"
#include "../application/modbus/ModbusPagePresenter.h"
#include "../widgets/BaseConnectionWidget.h"
#include "../widgets/FunctionWidget.h"
#include "../widgets/TrafficMonitorWidget.h"
#include "../widgets/ControlWidget.h"
#include "../widgets/CollapsibleSection.h"
#include "../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace ui::views {

BaseModbusPage::BaseModbusPage(ui::application::modbus::SessionMode mode,
                               ui::common::ISettingsService* settingsService,
                               QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService),
      sessionMode_(mode) {
}

BaseModbusPage::~BaseModbusPage() noexcept = default;

void BaseModbusPage::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    if (pagePresenter_) {
        ui::application::modbus::ModbusTimingParams params;
        params.timeout = std::chrono::milliseconds(timeoutMs);
        params.retryCount = retries;
        params.retryInterval = std::chrono::milliseconds(retryIntervalMs);
        pagePresenter_->updateSettings(params);
    }
}

void BaseModbusPage::setLinked(bool linked) {
    if (pagePresenter_) {
        pagePresenter_->setLinked(linked);
    }
}

bool BaseModbusPage::isLinked() const {
    return pagePresenter_ && pagePresenter_->isLinked();
}

void BaseModbusPage::appendTrafficData(bool isTx, const QByteArray& data) {
    if (isTx) {
        appendSendData(data);
    } else {
        appendReceiveData(data);
    }
}

void BaseModbusPage::setupUi(ui::widgets::BaseConnectionWidget* connWidget) {
    connectionWidget_ = connWidget;
    const auto descriptor = ui::application::modbus::modeDescriptor(sessionMode_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);

    mainLayout_->addWidget(connectionWidget_);

    const QString prefix = QString::fromLatin1(descriptor.settingsPrefix);

    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup(prefix + QStringLiteral("/standard"));
    functionWidget_->setTransportMode(descriptor.transportUiMode);
    mainLayout_->addWidget(functionWidget_);

    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey(prefix + QStringLiteral("/ui/dataMonitorCollapsed"));
    auto dataLayout = new QHBoxLayout(dataGroup_->contentWidget());
    dataLayout->setContentsMargins(4, 0, 4, 4);
    dataLayout->setSpacing(4);

    receiveGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto receiveLayout = new QVBoxLayout(receiveGroup_);
    auto receiveToolbar = new QHBoxLayout();
    copyReceiveButton_ = new QPushButton(receiveGroup_);
    clearReceiveButton_ = new QPushButton(receiveGroup_);
    receiveToolbar->addStretch();
    receiveToolbar->addWidget(copyReceiveButton_);
    receiveToolbar->addWidget(clearReceiveButton_);
    receiveTextEdit_ = new QTextEdit(receiveGroup_);
    receiveTextEdit_->setReadOnly(true);
    receiveTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    receiveLayout->addLayout(receiveToolbar);
    receiveLayout->addWidget(receiveTextEdit_);

    sendGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto sendLayout = new QVBoxLayout(sendGroup_);
    auto sendToolbar = new QHBoxLayout();
    copySendButton_ = new QPushButton(sendGroup_);
    clearSendButton_ = new QPushButton(sendGroup_);
    sendToolbar->addStretch();
    sendToolbar->addWidget(copySendButton_);
    sendToolbar->addWidget(clearSendButton_);
    sendTextEdit_ = new QTextEdit(sendGroup_);
    sendTextEdit_->setReadOnly(true);
    sendTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    sendLayout->addLayout(sendToolbar);
    sendLayout->addWidget(sendTextEdit_);

    dataLayout->addWidget(receiveGroup_, 1);
    dataLayout->addWidget(sendGroup_, 1);

    trafficMonitor_ = new widgets::TrafficMonitorWidget(settingsService_, this);
    trafficMonitor_->setMinimumHeight(140);
    trafficMonitor_->setSettingsGroup(prefix + QStringLiteral("/traffic"));

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);

    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup(prefix + QStringLiteral("/control"));
    mainLayout_->addWidget(controlWidget_);

    // Create the Presenter (composition root) and wire backend services.
    pagePresenter_ = new ui::application::modbus::ModbusPagePresenter(
        this, sessionMode_, this);
    pagePresenter_->setup(connectionWidget_, controlWidget_,
                          functionWidget_, trafficMonitor_);
    sessionPresenter_ = pagePresenter_->sessionPresenter();

    // Forward Presenter linkage signals to our own (consumed by
    // AnalyzerLinkCoordinator and other external consumers).
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageDataReceived,
            this, &BaseModbusPage::linkageDataReceived);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageToggled,
            this, &BaseModbusPage::linkageToggled);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageSourceDisconnected,
            this, &BaseModbusPage::linkageSourceDisconnected);

    setupViewOnlyConnections();

    mainLayout_->addStretch();

    retranslateUi();
}

void BaseModbusPage::setupViewOnlyConnections() {
    if (controlWidget_) {
        auto ensureConnected = [this]() {
            if (pagePresenter_ && pagePresenter_->isSessionConnected()) {
                return true;
            }
            ui::common::ConnectionAlert::showNotConnected(this);
            return false;
        };
        controlWidget_->setConnectionValidator(ensureConnected);
    }

    connect(clearReceiveButton_, &QPushButton::clicked, this, [this]() {
        lastReceiveFrame_.clear();
        if (receiveTextEdit_) receiveTextEdit_->clear();
    });
    connect(clearSendButton_, &QPushButton::clicked, this, [this]() {
        lastSendFrame_.clear();
        if (sendTextEdit_) sendTextEdit_->clear();
    });
    connect(copyReceiveButton_, &QPushButton::clicked, this, [this]() {
        if (!receiveTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(receiveTextEdit_->toPlainText());
    });
    connect(copySendButton_, &QPushButton::clicked, this, [this]() {
        if (!sendTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(sendTextEdit_->toPlainText());
    });
}

QString BaseModbusPage::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void BaseModbusPage::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void BaseModbusPage::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void BaseModbusPage::refreshReceiveDisplay() {
    if (!receiveTextEdit_) return;
    if (lastReceiveFrame_.isEmpty()) {
        receiveTextEdit_->clear();
        return;
    }
    const bool hex = true;
    receiveTextEdit_->setPlainText(tr("[%1] %2")
                                       .arg(tr("RX"))
                                       .arg(formatData(lastReceiveFrame_, hex)));
}

void BaseModbusPage::refreshSendDisplay() {
    if (!sendTextEdit_) return;
    if (lastSendFrame_.isEmpty()) {
        sendTextEdit_->clear();
        return;
    }
    const bool hex = true;
    sendTextEdit_->setPlainText(tr("[%1] %2")
                                     .arg(tr("TX"))
                                     .arg(formatData(lastSendFrame_, hex)));
}

void BaseModbusPage::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
    refreshReceiveDisplay();
    refreshSendDisplay();
}

void BaseModbusPage::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

} // namespace ui::views
