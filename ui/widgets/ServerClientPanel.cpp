/**
 * @file ServerClientPanel.cpp
 * @brief Implementation of ServerClientPanel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ServerClientPanel.h"

#include <QCheckBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui::widgets {

namespace {

constexpr int kClientIdRole = Qt::UserRole + 1;
constexpr int kPeerInfoRole = Qt::UserRole + 2;

} // namespace

ServerClientPanel::ServerClientPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void ServerClientPanel::addOrUpdateClient(int clientId, const QString& peerInfo)
{
    QListWidgetItem* item = findClientItem(clientId);
    if (!item) {
        item = new QListWidgetItem(clientList_);
        item->setData(kClientIdRole, clientId);
    }

    item->setData(kPeerInfoRole, peerInfo);
    updateItemText(item);

    if (clientList_->selectedItems().isEmpty() && !broadcastEnabled()) {
        clientList_->setCurrentItem(item);
        item->setSelected(true);
    }

    refreshSummary();
    updateActionState();
}

bool ServerClientPanel::removeClient(int clientId)
{
    QListWidgetItem* item = findClientItem(clientId);
    if (!item) {
        return false;
    }

    const int row = clientList_->row(item);
    delete clientList_->takeItem(row);

    if (clientList_->count() > 0 && clientList_->selectedItems().isEmpty() && !broadcastEnabled()) {
        clientList_->setCurrentRow(0);
        if (clientList_->item(0)) {
            clientList_->item(0)->setSelected(true);
        }
    }

    if (clientList_->count() == 0 && broadcastCheck_) {
        broadcastCheck_->setChecked(false);
    }

    refreshSummary();
    updateActionState();
    return true;
}

void ServerClientPanel::clearClients()
{
    clientList_->clear();
    if (broadcastCheck_) {
        broadcastCheck_->setChecked(false);
    }
    refreshSummary();
    updateActionState();
}

QList<int> ServerClientPanel::selectedClientIds() const
{
    QList<int> clientIds;
    const QList<QListWidgetItem*> items = clientList_->selectedItems();
    clientIds.reserve(items.size());
    for (QListWidgetItem* item : items) {
        clientIds.append(item->data(kClientIdRole).toInt());
    }
    return clientIds;
}

QList<int> ServerClientPanel::allClientIds() const
{
    QList<int> clientIds;
    clientIds.reserve(clientList_->count());
    for (int index = 0; index < clientList_->count(); ++index) {
        if (QListWidgetItem* item = clientList_->item(index)) {
            clientIds.append(item->data(kClientIdRole).toInt());
        }
    }
    return clientIds;
}

bool ServerClientPanel::broadcastEnabled() const
{
    return broadcastCheck_ && broadcastCheck_->isChecked();
}

bool ServerClientPanel::hasClients() const
{
    return clientList_ && clientList_->count() > 0;
}

void ServerClientPanel::onSelectionChanged()
{
    refreshSummary();
    updateActionState();
}

void ServerClientPanel::onBroadcastToggled(bool checked)
{
    if (clientList_) {
        clientList_->setEnabled(!checked && clientList_->count() > 0);
    }
    refreshSummary();
    updateActionState();
}

void ServerClientPanel::onDisconnectSelectedClicked()
{
    const QList<int> clientIds = selectedClientIds();
    if (!clientIds.isEmpty()) {
        emit disconnectClientsRequested(clientIds);
    }
}

void ServerClientPanel::onDisconnectAllClicked()
{
    if (hasClients()) {
        emit disconnectAllClientsRequested();
    }
}

void ServerClientPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    summaryLabel_ = new QLabel(this);
    summaryLabel_->setObjectName(QStringLiteral("serverClientSummaryLabel"));
    summaryLabel_->setWordWrap(true);
    mainLayout->addWidget(summaryLabel_);

    selectionHintLabel_ = new QLabel(this);
    selectionHintLabel_->setObjectName(QStringLiteral("serverClientSelectionHintLabel"));
    selectionHintLabel_->setWordWrap(true);
    mainLayout->addWidget(selectionHintLabel_);

    broadcastCheck_ = new QCheckBox(this);
    broadcastCheck_->setObjectName(QStringLiteral("serverBroadcastCheck"));
    connect(broadcastCheck_, &QCheckBox::toggled, this, &ServerClientPanel::onBroadcastToggled);
    mainLayout->addWidget(broadcastCheck_);

    clientList_ = new QListWidget(this);
    clientList_->setObjectName(QStringLiteral("serverClientList"));
    clientList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    clientList_->setMinimumWidth(220);
    clientList_->setMinimumHeight(120);
    connect(clientList_, &QListWidget::itemSelectionChanged,
            this, &ServerClientPanel::onSelectionChanged);
    mainLayout->addWidget(clientList_, 1);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(6);

    disconnectSelectedButton_ = new QPushButton(this);
    disconnectSelectedButton_->setObjectName(QStringLiteral("serverDisconnectSelectedButton"));
    connect(disconnectSelectedButton_, &QPushButton::clicked,
            this, &ServerClientPanel::onDisconnectSelectedClicked);
    buttonLayout->addWidget(disconnectSelectedButton_);

    disconnectAllButton_ = new QPushButton(this);
    disconnectAllButton_->setObjectName(QStringLiteral("serverDisconnectAllButton"));
    connect(disconnectAllButton_, &QPushButton::clicked,
            this, &ServerClientPanel::onDisconnectAllClicked);
    buttonLayout->addWidget(disconnectAllButton_);

    mainLayout->addLayout(buttonLayout);

    retranslateUi();
    refreshSummary();
    updateActionState();
}

void ServerClientPanel::retranslateUi()
{
    if (selectionHintLabel_) {
        selectionHintLabel_->setText(tr("Select one or more clients for directed send, or enable broadcast."));
    }
    if (broadcastCheck_) {
        broadcastCheck_->setText(tr("Broadcast To All Clients"));
    }
    if (disconnectSelectedButton_) {
        disconnectSelectedButton_->setText(tr("Disconnect Selected"));
    }
    if (disconnectAllButton_) {
        disconnectAllButton_->setText(tr("Disconnect All Clients"));
    }

    for (int index = 0; index < clientList_->count(); ++index) {
        updateItemText(clientList_->item(index));
    }

    refreshSummary();
}

void ServerClientPanel::refreshSummary()
{
    if (!summaryLabel_) {
        return;
    }

    const int clientCount = clientList_ ? clientList_->count() : 0;
    const QString targetText = broadcastEnabled()
        ? tr("all clients")
        : tr("%1 selected").arg(selectedClientIds().size());
    summaryLabel_->setText(tr("Connected Clients: %1 | Send Target: %2")
                               .arg(clientCount)
                               .arg(targetText));
}

void ServerClientPanel::updateActionState()
{
    const bool hasAnyClients = hasClients();
    const bool hasSelection = !selectedClientIds().isEmpty();

    if (broadcastCheck_) {
        broadcastCheck_->setEnabled(hasAnyClients);
    }
    if (clientList_) {
        clientList_->setEnabled(hasAnyClients && !broadcastEnabled());
    }
    if (disconnectSelectedButton_) {
        disconnectSelectedButton_->setEnabled(hasSelection);
    }
    if (disconnectAllButton_) {
        disconnectAllButton_->setEnabled(hasAnyClients);
    }
}

void ServerClientPanel::updateItemText(QListWidgetItem* item) const
{
    if (!item) {
        return;
    }

    const int clientId = item->data(kClientIdRole).toInt();
    const QString peerInfo = item->data(kPeerInfoRole).toString();
    item->setText(tr("Client #%1 - %2").arg(clientId).arg(peerInfo));
}

QListWidgetItem* ServerClientPanel::findClientItem(int clientId) const
{
    if (!clientList_) {
        return nullptr;
    }

    for (int index = 0; index < clientList_->count(); ++index) {
        QListWidgetItem* item = clientList_->item(index);
        if (item && item->data(kClientIdRole).toInt() == clientId) {
            return item;
        }
    }
    return nullptr;
}

void ServerClientPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
