/**
 * @file UtilitiesView.cpp
 * @brief Implementation of UtilitiesView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UtilitiesView.h"
#include "views/converter/Ieee754ConverterWidget.h"
#include "views/converter/ModbusFrameBuilderWidget.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QEvent>

namespace ui::views::tools {

UtilitiesView::UtilitiesView(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

void UtilitiesView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setDocumentMode(true);

    ieee754Converter_ = new converter::Ieee754ConverterWidget(this);
    tabWidget_->addTab(ieee754Converter_, tr("IEEE 754 Converter"));

    frameBuilder_ = new converter::ModbusFrameBuilderWidget(settingsService_, this);
    tabWidget_->addTab(frameBuilder_, tr("Frame Builder"));

    layout->addWidget(tabWidget_);

    connect(frameBuilder_, &converter::ModbusFrameBuilderWidget::inspectInAnalyzerRequested,
            this, &UtilitiesView::inspectInAnalyzerRequested);
}

void UtilitiesView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void UtilitiesView::retranslateUi() {
    if (tabWidget_) {
        tabWidget_->setTabText(0, tr("IEEE 754 Converter"));
        tabWidget_->setTabText(1, tr("Frame Builder"));
    }
}

} // namespace ui::views::tools
