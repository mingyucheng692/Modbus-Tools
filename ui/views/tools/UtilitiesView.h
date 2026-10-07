// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

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

// Role: Shell-level container page for offline tools and converters (Side Navigation item 5).
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent tree (MainWindow); owns tab widgets.
class UtilitiesView : public QWidget {
    Q_OBJECT

public:
    explicit UtilitiesView(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~UtilitiesView() override = default;

signals:
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
