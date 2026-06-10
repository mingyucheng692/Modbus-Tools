/**
 * @file UpdateInteractionView.h
 * @brief Extracted IUpdateInteractionView implementation for MainWindow.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "application/UpdateCoordinator.h"
#include <QObject>

class QAction;
class QMenu;

namespace core::update { class UpdateManager; }

namespace ui {

class UpdateInteractionView : public QObject,
                              public application::IUpdateInteractionView {
    Q_OBJECT

public:
    explicit UpdateInteractionView(QObject* parent = nullptr);

    void setUpdateCheckAction(QAction* action);
    void setAboutMenu(QMenu* menu);

    // IUpdateInteractionView
    void setUpdateCheckActionEnabled(bool enabled) override;
    void setUpdateIndicatorVisible(bool visible) override;
    void showUpdateInfoMessage(const QString& title, const QString& message) override;
    void showUpdateWarningMessage(const QString& title, const QString& message) override;
    void showUpdateCriticalMessage(const QString& title, const QString& message) override;
    bool confirmOpenDownloadPage(const QString& latestVersion) override;
    application::UpdatePromptChoice promptUpdateAction(const QString& currentVersion, const QString& latestVersion) override;
    void showUpdateProgress(core::update::UpdateManager* updateManager) override;

private:
    QAction* updateCheckAction_ = nullptr;
    QMenu* aboutMenu_ = nullptr;
};

} // namespace ui