/**
 * @file UtilitiesView.h
 * @brief Container view hosting offline engineering utilities and calculators.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>

class QTabWidget;

namespace infra::config {
class ISettingsService;
}

namespace ui::views::converter {
class Ieee754ConverterWidget;
class ModbusFrameBuilderWidget;
}

namespace ui::views::tools {

/**
 * @brief Shell-level container page for offline tools and converters (Side Navigation item 5).
 *
 * Hosts a QTabWidget containing:
 * - Tab 0: IEEE 754 Floating-Point Converter & Endianness Matrix
 * - Tab 1: Visual Modbus RAW Frame Builder & Field Inspector
 *
 * Exposes inspectInAnalyzerRequested to forward cross-view linkage without
 * direct coupling to FrameAnalyzerWidget.
 */
class UtilitiesView : public QWidget {
    Q_OBJECT

public:
    explicit UtilitiesView(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~UtilitiesView() override = default;

signals:
    /**
     * @brief Relays the frame builder request to inspect the generated frame in Frame Analyzer.
     * @param hexText The generated hex frame string.
     */
    void inspectInAnalyzerRequested(const QString& hexText);

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void retranslateUi();

    infra::config::ISettingsService* settingsService_ = nullptr;
    QTabWidget* tabWidget_ = nullptr;
    converter::Ieee754ConverterWidget* ieee754Converter_ = nullptr;
    converter::ModbusFrameBuilderWidget* frameBuilder_ = nullptr;
};

} // namespace ui::views::tools
