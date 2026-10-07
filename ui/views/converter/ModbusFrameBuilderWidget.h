// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusAddressMapping.h"

class QComboBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QGroupBox;
class QTimer;

namespace infra::config {
class ISettingsService;
}

namespace ui::views::converter {

// Role: Visual builder for raw Modbus RTU, TCP, and ASCII frames with field breakdown.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy.
class ModbusFrameBuilderWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModbusFrameBuilderWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusFrameBuilderWidget() override = default;

signals:
    void inspectInAnalyzerRequested(const QString& hexText);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onProtocolChanged(int index);
    void onFunctionCodeChanged(int index);
    void onParameterChanged();
    void rebuildFrame();
    void onCopySpacedHexClicked();
    void onCopyCompactHexClicked();
    void onCopyCArrayClicked();
    void onInspectInAnalyzerClicked();

private:
    void setupUi();
    void retranslateUi();
    void loadSettings();
    void saveSettings();

    void updateFormVisibility();
    void populateFieldTable(int protocol, uint8_t slaveOrUnitId, ::modbus::base::FunctionCode fc,
                            uint16_t startPduAddr, int quantity, const QByteArray& adu);
    void copyToClipboard(const QString& text, const QString& message);

    infra::config::ISettingsService* settingsService_ = nullptr;

    // --- State ---
    bool isSyncing_ = false;
    QTimer* rebuildTimer_ = nullptr;
    QByteArray currentAdu_;
    QString currentSpacedHex_;

    // --- UI Controls ---
    // Parameters Group
    QGroupBox* paramGroup_ = nullptr;
    QLabel* protocolLabel_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QLabel* slaveIdLabel_ = nullptr;
    QLineEdit* slaveIdEdit_ = nullptr;
    QLabel* functionLabel_ = nullptr;
    QComboBox* functionCombo_ = nullptr;
    QLabel* addressBaseLabel_ = nullptr;
    QComboBox* addressBaseCombo_ = nullptr;
    QLabel* addressLabel_ = nullptr;
    QLineEdit* addressEdit_ = nullptr;
    QLabel* quantityLabel_ = nullptr;
    QSpinBox* quantitySpin_ = nullptr;
    QLabel* writeDataLabel_ = nullptr;
    QLineEdit* writeDataEdit_ = nullptr;
    QLabel* dataFormatLabel_ = nullptr;
    QComboBox* dataFormatCombo_ = nullptr;

    // Output & Breakdown Group
    QGroupBox* outputGroup_ = nullptr;
    QLabel* hexOutputLabel_ = nullptr;
    QLineEdit* hexOutputEdit_ = nullptr;
    QLabel* errorLabel_ = nullptr;
    QTableWidget* breakdownTable_ = nullptr;
    QPushButton* copySpacedBtn_ = nullptr;
    QPushButton* copyCompactBtn_ = nullptr;
    QPushButton* copyCArrayBtn_ = nullptr;
    QPushButton* inspectAnalyzerBtn_ = nullptr;
};

} // namespace ui::views::converter
