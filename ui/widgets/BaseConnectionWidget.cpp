/**
 * @file BaseConnectionWidget.cpp
 * @brief Implementation of BaseConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "BaseConnectionWidget.h"
#include "CollapsibleSection.h"
#include "infra/config/ISettingsService.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>

namespace ui::widgets {

BaseConnectionWidget::BaseConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

BaseConnectionWidget::~BaseConnectionWidget() = default;

void BaseConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + QStringLiteral("/ui/connectionSettingsCollapsed"));
    }
    loadSettings();
}

void BaseConnectionWidget::setDisplayState(DisplayState state) {
    displayState_ = state;
    isConnected_ = (state == DisplayState::TransportConnected
                    || state == DisplayState::Connected
                    || state == DisplayState::Listening
                    || state == DisplayState::Bound);
    applyDisplayState();
}

bool BaseConnectionWidget::inputsLocked(DisplayState state) noexcept {
    return state != DisplayState::Disconnected;
}

void BaseConnectionWidget::applyDisplayState() {
    // Template method: common skeleton shared by Serial and Network
    // connection widgets. Subclasses supply data via getStateDisplayInfo()
    // and per-widget enabling via applyInputWidgetsState(); protocol-specific
    // refresh is delegated to updateProtocolUi().
    const auto info = getStateDisplayInfo(displayState_);

    connectBtn_->setText(info.buttonText);
    statusLabel_->setText(info.statusText);
    statusLabel_->setStyleSheet(info.statusStyle);

    const bool enabled = !inputsLocked(displayState_);
    applyInputWidgetsState(enabled);
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);

    updateProtocolUi();
}

void BaseConnectionWidget::createCommonWidgets(QWidget* parent) {
    connectBtn_ = new QPushButton(parent);
    
    statusLabel_ = new QLabel(parent);
    statusLabel_->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));
}

void BaseConnectionWidget::setupCommonConnections() {
}

void BaseConnectionWidget::loadCommonSettings() {
}

void BaseConnectionWidget::saveCommonSettings() {
}

void BaseConnectionWidget::retranslateCommonUi() {
    if (section_) {
        section_->setTitle(tr("Connection Settings"));
    }
}

QHBoxLayout* BaseConnectionWidget::setupBaseUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    section_ = new CollapsibleSection(settingsService_, this);
    auto* layout = new QHBoxLayout(section_->contentWidget());
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);
    return layout;
}

void BaseConnectionWidget::finishBaseUi(const QString& sectionSettingsKey) {
    // Add common widgets to the layout (must be called after subclass has
    // added its widgets, since createCommonWidgets creates the widgets).
    createCommonWidgets(section_->contentWidget());
    auto* layout = qobject_cast<QHBoxLayout*>(section_->contentWidget()->layout());
    if (layout) {
        layout->addSpacing(4);
        layout->addWidget(connectBtn_);
        layout->addWidget(statusLabel_);
        layout->addStretch();
    }

    // Find the main layout and add the section
    auto* mainLayout = qobject_cast<QHBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(section_);
    }

    setupCommonConnections();
    loadSettings();

    if (!sectionSettingsKey.isEmpty()) {
        section_->setSettingsKey(sectionSettingsKey);
    }

    retranslateUi();
}

void BaseConnectionWidget::retranslateUi() {
    retranslateCommonUi();
    applyDisplayState();
}

void BaseConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
