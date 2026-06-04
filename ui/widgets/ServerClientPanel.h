/**
 * @file ServerClientPanel.h
 * @brief TCP server client management panel for selection, broadcast, and disconnect actions.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QList>

class QCheckBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QEvent;

namespace ui::widgets {

/**
 * @brief Displays connected TCP server clients and exposes server-side client actions.
 */
class ServerClientPanel : public QWidget {
    Q_OBJECT

public:
    explicit ServerClientPanel(QWidget* parent = nullptr);

    void addOrUpdateClient(int clientId, const QString& peerInfo);
    [[nodiscard]] bool removeClient(int clientId);
    void clearClients();

    [[nodiscard]] QList<int> selectedClientIds() const;
    [[nodiscard]] QList<int> allClientIds() const;
    [[nodiscard]] bool broadcastEnabled() const;
    [[nodiscard]] bool hasClients() const;

signals:
    void disconnectClientsRequested(const QList<int>& clientIds);
    void disconnectAllClientsRequested();

private slots:
    void onSelectionChanged();
    void onBroadcastToggled(bool checked);
    void onDisconnectSelectedClicked();
    void onDisconnectAllClicked();

private:
    void setupUi();
    void retranslateUi();
    void refreshSummary();
    void updateActionState();
    void updateItemText(QListWidgetItem* item) const;
    [[nodiscard]] QListWidgetItem* findClientItem(int clientId) const;
    void changeEvent(QEvent* event) override;

    QLabel* summaryLabel_ = nullptr;
    QLabel* selectionHintLabel_ = nullptr;
    QCheckBox* broadcastCheck_ = nullptr;
    QListWidget* clientList_ = nullptr;
    QPushButton* disconnectSelectedButton_ = nullptr;
    QPushButton* disconnectAllButton_ = nullptr;
};

} // namespace ui::widgets
