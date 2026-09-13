/**
 * @file UpdateCommandDialog.cpp
 * @brief Implements the terminal command update dialog (方案 C).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateCommandDialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace ui {

UpdateCommandDialog::UpdateCommandDialog(const QString& latestVersion,
                                         const QString& command,
                                         const QString& releaseUrl,
                                         QWidget* parent)
    : QDialog(parent),
      releaseUrl_(releaseUrl) {
    setWindowTitle(tr("Update to v%1").arg(latestVersion));
    setModal(true);
    setMinimumWidth(560);

    auto* hintLabel = new QLabel(tr("Quit Modbus-Tools before running step 3. "
                                    "Paste each block into a terminal, in order."), this);
    hintLabel->setWordWrap(true);

    commandEdit_ = new QPlainTextEdit(command, this);
    commandEdit_->setReadOnly(true);
    commandEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto* buttonLayout = new QHBoxLayout;
    copyButton_ = new QPushButton(tr("Copy Commands"), this);
    auto* openPageButton = new QPushButton(tr("Open Download Page"), this);
    auto* closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(copyButton_);
    buttonLayout->addWidget(openPageButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(hintLabel);
    mainLayout->addWidget(commandEdit_);
    mainLayout->addLayout(buttonLayout);

    connect(copyButton_, &QPushButton::clicked, this, &UpdateCommandDialog::copyCommands);
    connect(openPageButton, &QPushButton::clicked, this, &UpdateCommandDialog::openDownloadPage);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void UpdateCommandDialog::copyCommands() {
    QApplication::clipboard()->setText(commandEdit_->toPlainText());
    // Transient feedback so the user trusts the click landed.
    copyButton_->setText(tr("Copied!"));
    copyButton_->setEnabled(false);
    QTimer::singleShot(1500, this, [this] {
        copyButton_->setText(tr("Copy Commands"));
        copyButton_->setEnabled(true);
    });
}

void UpdateCommandDialog::openDownloadPage() {
    QDesktopServices::openUrl(QUrl(releaseUrl_));
    accept();
}

} // namespace ui
