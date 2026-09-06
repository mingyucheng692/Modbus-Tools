/**
 * @file Ieee754ConverterWidget.h
 * @brief Industrial IEEE 754 floating-point converter & endianness live matrix tool.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <cstdint>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"

class QLineEdit;
class QLabel;
class QTableWidget;
class QRadioButton;
class QButtonGroup;
class QPushButton;
class QGroupBox;

namespace ui::views::converter {

/**
 * @brief Interactive conversion workspace for IEEE 754 Float32 / Float64 values,
 *        four Modbus endianness permutations (ABCD, CDAB, BADC, DCBA), and
 *        bitfield breakdown.
 */
class Ieee754ConverterWidget : public QWidget {
    Q_OBJECT

public:
    enum class Precision {
        Float32,
        Float64
    };

    explicit Ieee754ConverterWidget(QWidget* parent = nullptr);
    ~Ieee754ConverterWidget() override = default;

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onPrecisionChanged(int id);
    void onFloatInputEdited();
    void onHexInputEdited();
    void onSignedIntInputEdited();
    void onUnsignedIntInputEdited();
    void onPresetClicked(double value, bool isSpecial = false, uint64_t specialRaw = 0);
    void onCopyHexClicked(int row);
    void onCopyWordsClicked(int row);

private:
    void setupUi();
    void retranslateUi();
    void updateAllViewsFromRaw();
    void updateInputFields();
    void updateMatrixTable();
    void updateBitfieldBreakdown();
    void copyToClipboard(const QString& text, const QString& successMsg);

    [[nodiscard]] QString formatBinaryWithSpaces(uint64_t val, int bitCount) const;
    [[nodiscard]] QString rawHexFormatted(const uint8_t* bytes, int count) const;

    // --- State ---
    Precision precision_ = Precision::Float32;
    uint64_t rawData_ = 0x42F6E979; ///< Default to 123.456f in Big-Endian
    bool isSyncing_ = false;

    // --- UI Controls ---
    QGroupBox* precisionGroup_ = nullptr;
    QButtonGroup* precisionBtnGroup_ = nullptr;
    QRadioButton* radioFloat32_ = nullptr;
    QRadioButton* radioFloat64_ = nullptr;

    // Quick Presets
    QGroupBox* presetsGroup_ = nullptr;
    QPushButton* maxNormalBtn_ = nullptr;
    QPushButton* infBtn_ = nullptr;
    QPushButton* nanBtn_ = nullptr;

    // Reactive Input Area
    QGroupBox* inputGroup_ = nullptr;
    QLabel* floatLabel_ = nullptr;
    QLineEdit* floatEdit_ = nullptr;
    QLabel* hexLabel_ = nullptr;
    QLineEdit* hexEdit_ = nullptr;
    QLabel* signedLabel_ = nullptr;
    QLineEdit* signedEdit_ = nullptr;
    QLabel* unsignedLabel_ = nullptr;
    QLineEdit* unsignedEdit_ = nullptr;
    QLabel* inputHintLabel_ = nullptr;

    // Endian Matrix Table
    QGroupBox* matrixGroup_ = nullptr;
    QTableWidget* matrixTable_ = nullptr;

    // Bitfield Breakdown
    QGroupBox* bitfieldGroup_ = nullptr;
    QLabel* binaryFullLabel_ = nullptr;
    QLabel* signBitLabel_ = nullptr;
    QLabel* expBitsLabel_ = nullptr;
    QLabel* fracBitsLabel_ = nullptr;
    QLabel* formulaLabel_ = nullptr;
};

} // namespace ui::views::converter
