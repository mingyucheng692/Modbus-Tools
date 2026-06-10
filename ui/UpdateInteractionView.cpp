/**
 * @file UpdateInteractionView.cpp
 * @brief Implementation of UpdateInteractionView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateInteractionView.h"
#include "../core/update/UpdateManager.h"
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>

namespace ui {

UpdateInteractionView::UpdateInteractionView(QObject* parent)
    : QObject(parent) {
    Q_ASSERT(qobject_cast<QWidget*>(parent));
}

void UpdateInteractionView::setUpdateCheckAction(QAction* action) {
    updateCheckAction_ = action;
}

void UpdateInteractionView::setAboutMenu(QMenu* menu) {
    aboutMenu_ = menu;
}

void UpdateInteractionView::setUpdateCheckActionEnabled(bool enabled) {
    if (updateCheckAction_) {
        updateCheckAction_->setEnabled(enabled);
    }
}

void UpdateInteractionView::setUpdateIndicatorVisible(bool visible) {
    if (aboutMenu_) {
        aboutMenu_->setTitle(visible ? tr("About") + " ●" : tr("About"));
    }
    if (updateCheckAction_) {
        updateCheckAction_->setText(visible ? tr("Check for Updates") + " ●" : tr("Check for Updates"));
    }
}

void UpdateInteractionView::showUpdateInfoMessage(const QString& title, const QString& message) {
    QMessageBox::information(qobject_cast<QWidget*>(parent()), title, message);
}

void UpdateInteractionView::showUpdateWarningMessage(const QString& title, const QString& message) {
    QMessageBox::warning(qobject_cast<QWidget*>(parent()), title, message);
}

void UpdateInteractionView::showUpdateCriticalMessage(const QString& title, const QString& message) {
    QMessageBox::critical(qobject_cast<QWidget*>(parent()), title, message);
}

bool UpdateInteractionView::confirmOpenDownloadPage(const QString& latestVersion) {
    return QMessageBox::question(
               qobject_cast<QWidget*>(parent()),
               tr("Update Available"),
               tr("New version v%1 available. Open download page?").arg(latestVersion))
           == QMessageBox::Yes;
}

application::UpdatePromptChoice UpdateInteractionView::promptUpdateAction(const QString& currentVersion, const QString& latestVersion) {
    auto* parentWidget = qobject_cast<QWidget*>(parent());
    QMessageBox mb(parentWidget);
    mb.setWindowTitle(tr("Update Available"));
    mb.setText(tr("Current: v%1, Latest: v%2\nChoose update method:").arg(currentVersion, latestVersion));
    auto* btnNow = mb.addButton(tr("Update Main Program"), QMessageBox::AcceptRole);
    auto* btnFull = mb.addButton(tr("Download Full Package"), QMessageBox::ActionRole);
    auto* btnCancel = mb.addButton(QMessageBox::Cancel);
    mb.exec();

    if (mb.clickedButton() == btnNow) {
        return application::UpdatePromptChoice::StartSilentUpdate;
    }
    if (mb.clickedButton() == btnFull) {
        return application::UpdatePromptChoice::DownloadFullPackage;
    }
    Q_UNUSED(btnCancel);
    return application::UpdatePromptChoice::Cancel;
}

void UpdateInteractionView::showUpdateProgress(core::update::UpdateManager* updateManager) {
    auto* parentWidget = qobject_cast<QWidget*>(parent());
    auto* progress = new QProgressDialog(tr("Downloading Update..."), tr("Cancel"), 0, 100, parentWidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setAutoClose(true);
    connect(updateManager, &core::update::UpdateManager::downloadProgress, progress, &QProgressDialog::setValue);
    connect(progress, &QProgressDialog::canceled, updateManager, &core::update::UpdateManager::cancelUpdate);
    connect(updateManager, &core::update::UpdateManager::updateReadyToInstall, this, [progress] { progress->close(); });
    connect(updateManager, &core::update::UpdateManager::updateFailed, this, [progress] { progress->close(); });
    connect(updateManager, &core::update::UpdateManager::updateCanceled, this, [progress] { progress->close(); });
    progress->show();
}

} // namespace ui