/**
 * @file Ieee754ConverterWidget.cpp
 * @brief Implementation of Ieee754ConverterWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "views/converter/Ieee754ConverterWidget.h"
#include "modbus/base/RegisterValueDecoder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QClipboard>
#include <QGuiApplication>
#include <QToolTip>
#include <QEvent>
#include <cmath>
#include <cstring>
#include <limits>

namespace ui::views::converter {

namespace {

// Helper to format float with high precision
QString formatDouble(double val, int sigDigits = 9) {
    if (std::isnan(val)) {
        return QStringLiteral("NaN");
    }
    if (std::isinf(val)) {
        return std::signbit(val) ? QStringLiteral("-Infinity") : QStringLiteral("+Infinity");
    }
    return modbus::codec::formatFloatScientific(val, sigDigits);
}

} // namespace

Ieee754ConverterWidget::Ieee754ConverterWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    updateAllViewsFromRaw();
}

void Ieee754ConverterWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // 1. Top Bar: Precision Selector & Presets
    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    precisionGroup_ = new QGroupBox(tr("Precision"), this);
    auto* precisionLayout = new QHBoxLayout(precisionGroup_);
    precisionLayout->setContentsMargins(10, 6, 10, 6);
    precisionLayout->setSpacing(12);

    radioFloat32_ = new QRadioButton(tr("Float32 (32-bit, 2 Regs)"), precisionGroup_);
    radioFloat64_ = new QRadioButton(tr("Float64 (64-bit, 4 Regs)"), precisionGroup_);
    radioFloat32_->setChecked(true);

    precisionBtnGroup_ = new QButtonGroup(this);
    precisionBtnGroup_->addButton(radioFloat32_, 0);
    precisionBtnGroup_->addButton(radioFloat64_, 1);
    precisionLayout->addWidget(radioFloat32_);
    precisionLayout->addWidget(radioFloat64_);

    connect(precisionBtnGroup_, &QButtonGroup::idClicked, this, &Ieee754ConverterWidget::onPrecisionChanged);
    topLayout->addWidget(precisionGroup_);

    // Presets
    presetsGroup_ = new QGroupBox(tr("Quick Presets"), this);
    auto* presetsLayout = new QHBoxLayout(presetsGroup_);
    presetsLayout->setContentsMargins(10, 6, 10, 6);
    presetsLayout->setSpacing(6);

    auto addPreset = [this, presetsLayout](const QString& label, double val, bool isSpecial = false, uint64_t specialRaw = 0) {
        auto* btn = new QPushButton(label, presetsGroup_);
        btn->setFixedHeight(24);
        connect(btn, &QPushButton::clicked, this, [this, val, isSpecial, specialRaw]() {
            onPresetClicked(val, isSpecial, specialRaw);
        });
        presetsLayout->addWidget(btn);
    };

    addPreset(QStringLiteral("0.0"), 0.0);
    addPreset(QStringLiteral("1.0"), 1.0);
    addPreset(QStringLiteral("-1.0"), -1.0);
    addPreset(QStringLiteral("100.0"), 100.0);
    addPreset(QStringLiteral("123.456"), 123.456);
    addPreset(QStringLiteral("π"), 3.141592653589793);
    addPreset(QStringLiteral("Max Normal"), std::numeric_limits<float>::max());
    addPreset(QStringLiteral("+Infinity"), std::numeric_limits<double>::infinity());
    addPreset(QStringLiteral("NaN"), std::numeric_limits<double>::quiet_NaN());

    presetsLayout->addStretch();
    topLayout->addWidget(presetsGroup_, 1);
    mainLayout->addLayout(topLayout);

    // 2. Interactive Reactive Inputs
    inputGroup_ = new QGroupBox(tr("Interactive Inputs (Two-Way Live Sync)"), this);
    auto* inputGridLayout = new QGridLayout(inputGroup_);
    inputGridLayout->setContentsMargins(12, 10, 12, 10);
    inputGridLayout->setSpacing(8);

    floatLabel_ = new QLabel(tr("Floating Point:"), inputGroup_);
    floatEdit_ = new QLineEdit(inputGroup_);
    floatEdit_->setPlaceholderText(tr("e.g. 123.456, -1.0, 1e-5, NaN, Inf"));
    inputGridLayout->addWidget(floatLabel_, 0, 0);
    inputGridLayout->addWidget(floatEdit_, 0, 1);

    hexLabel_ = new QLabel(tr("Hex (Big-Endian):"), inputGroup_);
    hexEdit_ = new QLineEdit(inputGroup_);
    hexEdit_->setPlaceholderText(tr("e.g. 42 F6 E9 79 or 42F6E979"));
    inputGridLayout->addWidget(hexLabel_, 0, 2);
    inputGridLayout->addWidget(hexEdit_, 0, 3);

    signedLabel_ = new QLabel(tr("Signed Integer:"), inputGroup_);
    signedEdit_ = new QLineEdit(inputGroup_);
    inputGridLayout->addWidget(signedLabel_, 1, 0);
    inputGridLayout->addWidget(signedEdit_, 1, 1);

    unsignedLabel_ = new QLabel(tr("Unsigned Integer:"), inputGroup_);
    unsignedEdit_ = new QLineEdit(inputGroup_);
    inputGridLayout->addWidget(unsignedLabel_, 1, 2);
    inputGridLayout->addWidget(unsignedEdit_, 1, 3);

    inputHintLabel_ = new QLabel(inputGroup_);
    inputHintLabel_->setStyleSheet(QStringLiteral("color: gray; font-size: 11px;"));
    inputHintLabel_->setText(tr("Editing any field instantly derives all others in real-time."));
    inputGridLayout->addWidget(inputHintLabel_, 2, 0, 1, 4);

    connect(floatEdit_, &QLineEdit::textEdited, this, &Ieee754ConverterWidget::onFloatInputEdited);
    connect(hexEdit_, &QLineEdit::textEdited, this, &Ieee754ConverterWidget::onHexInputEdited);
    connect(signedEdit_, &QLineEdit::textEdited, this, &Ieee754ConverterWidget::onSignedIntInputEdited);
    connect(unsignedEdit_, &QLineEdit::textEdited, this, &Ieee754ConverterWidget::onUnsignedIntInputEdited);

    mainLayout->addWidget(inputGroup_);

    // 3. Modbus Endian Matrix Table
    matrixGroup_ = new QGroupBox(tr("Four-Endian Live Matrix (Modbus Permutations)"), this);
    auto* matrixLayout = new QVBoxLayout(matrixGroup_);
    matrixLayout->setContentsMargins(8, 8, 8, 8);

    matrixTable_ = new QTableWidget(4, 7, matrixGroup_);
    matrixTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    matrixTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    matrixTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    matrixTable_->verticalHeader()->setVisible(false);
    matrixTable_->horizontalHeader()->setStretchLastSection(true);
    matrixTable_->setFixedHeight(160);

    matrixLayout->addWidget(matrixTable_);
    mainLayout->addWidget(matrixGroup_);

    // 4. IEEE 754 Bitfield Breakdown
    bitfieldGroup_ = new QGroupBox(tr("IEEE 754 Binary Bitfield Breakdown"), this);
    auto* bitfieldLayout = new QVBoxLayout(bitfieldGroup_);
    bitfieldLayout->setContentsMargins(12, 10, 12, 10);
    bitfieldLayout->setSpacing(6);

    binaryFullLabel_ = new QLabel(bitfieldGroup_);
    binaryFullLabel_->setStyleSheet(QStringLiteral("font-family: monospace; font-size: 13px; font-weight: bold;"));
    bitfieldLayout->addWidget(binaryFullLabel_);

    auto* fieldsLayout = new QHBoxLayout();
    fieldsLayout->setSpacing(8);

    signBitLabel_ = new QLabel(bitfieldGroup_);
    signBitLabel_->setStyleSheet(QStringLiteral("padding: 4px 8px; border-radius: 4px; background: rgba(59, 130, 246, 0.15); border: 1px solid #3B82F6; font-size: 11px;"));
    fieldsLayout->addWidget(signBitLabel_);

    expBitsLabel_ = new QLabel(bitfieldGroup_);
    expBitsLabel_->setStyleSheet(QStringLiteral("padding: 4px 8px; border-radius: 4px; background: rgba(16, 185, 129, 0.15); border: 1px solid #10B981; font-size: 11px;"));
    fieldsLayout->addWidget(expBitsLabel_);

    fracBitsLabel_ = new QLabel(bitfieldGroup_);
    fracBitsLabel_->setStyleSheet(QStringLiteral("padding: 4px 8px; border-radius: 4px; background: rgba(245, 158, 11, 0.15); border: 1px solid #F59E0B; font-size: 11px;"));
    fieldsLayout->addWidget(fracBitsLabel_, 1);

    bitfieldLayout->addLayout(fieldsLayout);

    formulaLabel_ = new QLabel(bitfieldGroup_);
    formulaLabel_->setStyleSheet(QStringLiteral("color: gray; font-size: 11px; margin-top: 4px;"));
    bitfieldLayout->addWidget(formulaLabel_);

    mainLayout->addWidget(bitfieldGroup_);

    retranslateUi();
}

void Ieee754ConverterWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void Ieee754ConverterWidget::retranslateUi()
{
    if (precisionGroup_) precisionGroup_->setTitle(tr("Precision"));
    if (radioFloat32_) radioFloat32_->setText(tr("Float32 (32-bit, 2 Regs)"));
    if (radioFloat64_) radioFloat64_->setText(tr("Float64 (64-bit, 4 Regs)"));
    if (presetsGroup_) presetsGroup_->setTitle(tr("Quick Presets"));
    if (inputGroup_) inputGroup_->setTitle(tr("Interactive Inputs (Two-Way Live Sync)"));
    if (floatLabel_) floatLabel_->setText(tr("Floating Point:"));
    if (hexLabel_) hexLabel_->setText(tr("Hex (Big-Endian):"));
    if (signedLabel_) signedLabel_->setText(precision_ == Precision::Float32 ? tr("Signed Int32:") : tr("Signed Int64:"));
    if (unsignedLabel_) unsignedLabel_->setText(precision_ == Precision::Float32 ? tr("Unsigned UInt32:") : tr("Unsigned UInt64:"));
    if (inputHintLabel_) inputHintLabel_->setText(tr("Editing any field instantly derives all others in real-time."));
    if (matrixGroup_) matrixGroup_->setTitle(tr("Four-Endian Live Matrix (Modbus Permutations)"));
    if (bitfieldGroup_) bitfieldGroup_->setTitle(tr("IEEE 754 Binary Bitfield Breakdown"));

    if (matrixTable_) {
        matrixTable_->setHorizontalHeaderLabels({
            tr("Endian Order"),
            tr("Raw Hex"),
            tr("Modbus Register Words"),
            tr("Floating Value"),
            tr("Signed Int"),
            tr("Unsigned Int"),
            tr("Actions")
        });
        matrixTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        matrixTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        matrixTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        matrixTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        matrixTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        matrixTable_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        matrixTable_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    }

    updateAllViewsFromRaw();
}

void Ieee754ConverterWidget::onPrecisionChanged(int id)
{
    if (id == 0) {
        precision_ = Precision::Float32;
        // Clamp rawData_ to 32-bit representation of 123.456f
        float fVal = 123.456f;
        uint32_t u32 = 0;
        std::memcpy(&u32, &fVal, 4);
        rawData_ = u32;
    } else {
        precision_ = Precision::Float64;
        double dVal = 123.456;
        uint64_t u64 = 0;
        std::memcpy(&u64, &dVal, 8);
        rawData_ = u64;
    }

    if (signedLabel_) signedLabel_->setText(precision_ == Precision::Float32 ? tr("Signed Int32:") : tr("Signed Int64:"));
    if (unsignedLabel_) unsignedLabel_->setText(precision_ == Precision::Float32 ? tr("Unsigned UInt32:") : tr("Unsigned UInt64:"));

    updateAllViewsFromRaw();
}

void Ieee754ConverterWidget::onFloatInputEdited()
{
    if (isSyncing_) return;
    isSyncing_ = true;

    const QString text = floatEdit_->text().trimmed();
    bool ok = false;

    if (precision_ == Precision::Float32) {
        float val = 0.0f;
        if (text.compare(QStringLiteral("nan"), Qt::CaseInsensitive) == 0) {
            val = std::numeric_limits<float>::quiet_NaN();
            ok = true;
        } else if (text.compare(QStringLiteral("+inf"), Qt::CaseInsensitive) == 0 ||
                   text.compare(QStringLiteral("inf"), Qt::CaseInsensitive) == 0) {
            val = std::numeric_limits<float>::infinity();
            ok = true;
        } else if (text.compare(QStringLiteral("-inf"), Qt::CaseInsensitive) == 0) {
            val = -std::numeric_limits<float>::infinity();
            ok = true;
        } else {
            val = text.toFloat(&ok);
        }

        if (ok) {
            uint32_t u32 = 0;
            std::memcpy(&u32, &val, 4);
            rawData_ = u32;
        }
    } else {
        double val = 0.0;
        if (text.compare(QStringLiteral("nan"), Qt::CaseInsensitive) == 0) {
            val = std::numeric_limits<double>::quiet_NaN();
            ok = true;
        } else if (text.compare(QStringLiteral("+inf"), Qt::CaseInsensitive) == 0 ||
                   text.compare(QStringLiteral("inf"), Qt::CaseInsensitive) == 0) {
            val = std::numeric_limits<double>::infinity();
            ok = true;
        } else if (text.compare(QStringLiteral("-inf"), Qt::CaseInsensitive) == 0) {
            val = -std::numeric_limits<double>::infinity();
            ok = true;
        } else {
            val = text.toDouble(&ok);
        }

        if (ok) {
            uint64_t u64 = 0;
            std::memcpy(&u64, &val, 8);
            rawData_ = u64;
        }
    }

    if (ok) {
        // Keep floatEdit_ as is to preserve user typing, update remaining controls
        uint8_t bytes[8];
        if (precision_ == Precision::Float32) {
            bytes[0] = static_cast<uint8_t>((rawData_ >> 24) & 0xFF);
            bytes[1] = static_cast<uint8_t>((rawData_ >> 16) & 0xFF);
            bytes[2] = static_cast<uint8_t>((rawData_ >> 8) & 0xFF);
            bytes[3] = static_cast<uint8_t>(rawData_ & 0xFF);
            hexEdit_->setText(rawHexFormatted(bytes, 4));
            int32_t s32 = 0;
            std::memcpy(&s32, &rawData_, 4);
            signedEdit_->setText(QString::number(s32));
            unsignedEdit_->setText(QString::number(static_cast<uint32_t>(rawData_)));
        } else {
            for (int i = 0; i < 8; ++i) {
                bytes[i] = static_cast<uint8_t>((rawData_ >> ((7 - i) * 8)) & 0xFF);
            }
            hexEdit_->setText(rawHexFormatted(bytes, 8));
            int64_t s64 = 0;
            std::memcpy(&s64, &rawData_, 8);
            signedEdit_->setText(QString::number(s64));
            unsignedEdit_->setText(QString::number(rawData_));
        }

        updateMatrixTable();
        updateBitfieldBreakdown();
    }

    isSyncing_ = false;
}

void Ieee754ConverterWidget::onHexInputEdited()
{
    if (isSyncing_) return;
    isSyncing_ = true;

    QString clean = hexEdit_->text().remove(QLatin1Char(' ')).remove(QStringLiteral("0x"), Qt::CaseInsensitive);
    bool ok = false;

    if (precision_ == Precision::Float32) {
        uint32_t u32 = clean.toUInt(&ok, 16);
        if (ok) {
            rawData_ = u32;
        }
    } else {
        uint64_t u64 = clean.toULongLong(&ok, 16);
        if (ok) {
            rawData_ = u64;
        }
    }

    if (ok) {
        // Update float, signed, unsigned
        if (precision_ == Precision::Float32) {
            float fVal = 0.0f;
            std::memcpy(&fVal, &rawData_, 4);
            floatEdit_->setText(formatDouble(fVal, 7));
            int32_t s32 = 0;
            std::memcpy(&s32, &rawData_, 4);
            signedEdit_->setText(QString::number(s32));
            unsignedEdit_->setText(QString::number(static_cast<uint32_t>(rawData_)));
        } else {
            double dVal = 0.0;
            std::memcpy(&dVal, &rawData_, 8);
            floatEdit_->setText(formatDouble(dVal, 15));
            int64_t s64 = 0;
            std::memcpy(&s64, &rawData_, 8);
            signedEdit_->setText(QString::number(s64));
            unsignedEdit_->setText(QString::number(rawData_));
        }

        updateMatrixTable();
        updateBitfieldBreakdown();
    }

    isSyncing_ = false;
}

void Ieee754ConverterWidget::onSignedIntInputEdited()
{
    if (isSyncing_) return;
    isSyncing_ = true;

    bool ok = false;
    if (precision_ == Precision::Float32) {
        int32_t s32 = signedEdit_->text().trimmed().toInt(&ok);
        if (ok) {
            uint32_t u32 = 0;
            std::memcpy(&u32, &s32, 4);
            rawData_ = u32;
        }
    } else {
        int64_t s64 = signedEdit_->text().trimmed().toLongLong(&ok);
        if (ok) {
            uint64_t u64 = 0;
            std::memcpy(&u64, &s64, 8);
            rawData_ = u64;
        }
    }

    if (ok) {
        updateInputFields();
        updateMatrixTable();
        updateBitfieldBreakdown();
    }

    isSyncing_ = false;
}

void Ieee754ConverterWidget::onUnsignedIntInputEdited()
{
    if (isSyncing_) return;
    isSyncing_ = true;

    bool ok = false;
    if (precision_ == Precision::Float32) {
        uint32_t u32 = unsignedEdit_->text().trimmed().toUInt(&ok);
        if (ok) {
            rawData_ = u32;
        }
    } else {
        uint64_t u64 = unsignedEdit_->text().trimmed().toULongLong(&ok);
        if (ok) {
            rawData_ = u64;
        }
    }

    if (ok) {
        updateInputFields();
        updateMatrixTable();
        updateBitfieldBreakdown();
    }

    isSyncing_ = false;
}

void Ieee754ConverterWidget::onPresetClicked(double value, bool isSpecial, uint64_t specialRaw)
{
    if (isSpecial) {
        rawData_ = specialRaw;
    } else if (precision_ == Precision::Float32) {
        float f = static_cast<float>(value);
        uint32_t u32 = 0;
        std::memcpy(&u32, &f, 4);
        rawData_ = u32;
    } else {
        uint64_t u64 = 0;
        std::memcpy(&u64, &value, 8);
        rawData_ = u64;
    }

    updateAllViewsFromRaw();
}

void Ieee754ConverterWidget::updateAllViewsFromRaw()
{
    isSyncing_ = true;
    updateInputFields();
    updateMatrixTable();
    updateBitfieldBreakdown();
    isSyncing_ = false;
}

void Ieee754ConverterWidget::updateInputFields()
{
    uint8_t bytes[8];
    if (precision_ == Precision::Float32) {
        float fVal = 0.0f;
        std::memcpy(&fVal, &rawData_, 4);
        floatEdit_->setText(formatDouble(fVal, 7));

        bytes[0] = static_cast<uint8_t>((rawData_ >> 24) & 0xFF);
        bytes[1] = static_cast<uint8_t>((rawData_ >> 16) & 0xFF);
        bytes[2] = static_cast<uint8_t>((rawData_ >> 8) & 0xFF);
        bytes[3] = static_cast<uint8_t>(rawData_ & 0xFF);
        hexEdit_->setText(rawHexFormatted(bytes, 4));

        int32_t s32 = 0;
        std::memcpy(&s32, &rawData_, 4);
        signedEdit_->setText(QString::number(s32));
        unsignedEdit_->setText(QString::number(static_cast<uint32_t>(rawData_)));
    } else {
        double dVal = 0.0;
        std::memcpy(&dVal, &rawData_, 8);
        floatEdit_->setText(formatDouble(dVal, 15));

        for (int i = 0; i < 8; ++i) {
            bytes[i] = static_cast<uint8_t>((rawData_ >> ((7 - i) * 8)) & 0xFF);
        }
        hexEdit_->setText(rawHexFormatted(bytes, 8));

        int64_t s64 = 0;
        std::memcpy(&s64, &rawData_, 8);
        signedEdit_->setText(QString::number(s64));
        unsignedEdit_->setText(QString::number(rawData_));
    }
}

void Ieee754ConverterWidget::updateMatrixTable()
{
    if (!matrixTable_) return;

    struct OrderSpec {
        modbus::base::RegisterOrder order;
        QString name;
        QString desc;
    };

    const OrderSpec specs[4] = {
        { modbus::base::RegisterOrder::ABCD, QStringLiteral("ABCD"), tr("Big-Endian (Standard / High-Word First)") },
        { modbus::base::RegisterOrder::CDAB, QStringLiteral("CDAB"), tr("Word Swap (Little-Endian Words / Middle-Endian)") },
        { modbus::base::RegisterOrder::BADC, QStringLiteral("BADC"), tr("Byte Swap (Word-Level Endian Inversion)") },
        { modbus::base::RegisterOrder::DCBA, QStringLiteral("DCBA"), tr("Little-Endian (True Little-Endian / Low-Word First)") }
    };

    uint8_t src[8];
    const int count = (precision_ == Precision::Float32) ? 4 : 8;
    if (precision_ == Precision::Float32) {
        src[0] = static_cast<uint8_t>((rawData_ >> 24) & 0xFF);
        src[1] = static_cast<uint8_t>((rawData_ >> 16) & 0xFF);
        src[2] = static_cast<uint8_t>((rawData_ >> 8) & 0xFF);
        src[3] = static_cast<uint8_t>(rawData_ & 0xFF);
    } else {
        for (int i = 0; i < 8; ++i) {
            src[i] = static_cast<uint8_t>((rawData_ >> ((7 - i) * 8)) & 0xFF);
        }
    }

    for (int r = 0; r < 4; ++r) {
        uint8_t buf[8];
        if (precision_ == Precision::Float32) {
            switch (specs[r].order) {
            case modbus::base::RegisterOrder::ABCD:
                buf[0] = src[0]; buf[1] = src[1]; buf[2] = src[2]; buf[3] = src[3];
                break;
            case modbus::base::RegisterOrder::CDAB:
                buf[0] = src[2]; buf[1] = src[3]; buf[2] = src[0]; buf[3] = src[1];
                break;
            case modbus::base::RegisterOrder::BADC:
                buf[0] = src[1]; buf[1] = src[0]; buf[2] = src[3]; buf[3] = src[2];
                break;
            case modbus::base::RegisterOrder::DCBA:
                buf[0] = src[3]; buf[1] = src[2]; buf[2] = src[1]; buf[3] = src[0];
                break;
            }
        } else {
            switch (specs[r].order) {
            case modbus::base::RegisterOrder::ABCD:
                std::memcpy(buf, src, 8);
                break;
            case modbus::base::RegisterOrder::CDAB:
                buf[0] = src[6]; buf[1] = src[7];
                buf[2] = src[4]; buf[3] = src[5];
                buf[4] = src[2]; buf[5] = src[3];
                buf[6] = src[0]; buf[7] = src[1];
                break;
            case modbus::base::RegisterOrder::BADC:
                buf[0] = src[1]; buf[1] = src[0];
                buf[2] = src[3]; buf[3] = src[2];
                buf[4] = src[5]; buf[5] = src[4];
                buf[6] = src[7]; buf[7] = src[6];
                break;
            case modbus::base::RegisterOrder::DCBA:
                for (int i = 0; i < 8; ++i) buf[i] = src[7 - i];
                break;
            }
        }

        // Column 0: Order
        auto* item0 = new QTableWidgetItem(QStringLiteral("%1 (%2)").arg(specs[r].name, specs[r].desc));
        item0->setFont(QFont(QStringLiteral("sans-serif"), 9, QFont::Bold));
        matrixTable_->setItem(r, 0, item0);

        // Column 1: Hex
        const QString hexStr = rawHexFormatted(buf, count);
        auto* item1 = new QTableWidgetItem(hexStr);
        item1->setFont(QFont(QStringLiteral("monospace"), 9));
        matrixTable_->setItem(r, 1, item1);

        // Column 2: Words
        QStringList wordsList;
        for (int i = 0; i < count / 2; ++i) {
            uint16_t w = (static_cast<uint16_t>(buf[i * 2]) << 8) | static_cast<uint16_t>(buf[i * 2 + 1]);
            wordsList << QStringLiteral("R%1: %2").arg(i).arg(QString::number(w, 16).toUpper().rightJustified(4, QLatin1Char('0')));
        }
        auto* item2 = new QTableWidgetItem(wordsList.join(QStringLiteral(", ")));
        item2->setFont(QFont(QStringLiteral("monospace"), 9));
        matrixTable_->setItem(r, 2, item2);

        // Values derived from this permutation
        QString floatStr;
        QString signedStr;
        QString unsignedStr;

        if (precision_ == Precision::Float32) {
            uint32_t uVal = (static_cast<uint32_t>(buf[0]) << 24) |
                            (static_cast<uint32_t>(buf[1]) << 16) |
                            (static_cast<uint32_t>(buf[2]) << 8)  |
                            static_cast<uint32_t>(buf[3]);
            float f = 0.0f;
            std::memcpy(&f, &uVal, 4);
            floatStr = formatDouble(f, 7);
            int32_t s = 0;
            std::memcpy(&s, &uVal, 4);
            signedStr = QString::number(s);
            unsignedStr = QString::number(uVal);
        } else {
            uint64_t uVal = 0;
            for (int i = 0; i < 8; ++i) {
                uVal = (uVal << 8) | static_cast<uint64_t>(buf[i]);
            }
            double d = 0.0;
            std::memcpy(&d, &uVal, 8);
            floatStr = formatDouble(d, 15);
            int64_t s = 0;
            std::memcpy(&s, &uVal, 8);
            signedStr = QString::number(s);
            unsignedStr = QString::number(uVal);
        }

        // Column 3: Float
        auto* item3 = new QTableWidgetItem(floatStr);
        item3->setFont(QFont(QStringLiteral("monospace"), 9));
        matrixTable_->setItem(r, 3, item3);

        // Column 4: Signed
        auto* item4 = new QTableWidgetItem(signedStr);
        item4->setFont(QFont(QStringLiteral("monospace"), 9));
        matrixTable_->setItem(r, 4, item4);

        // Column 5: Unsigned
        auto* item5 = new QTableWidgetItem(unsignedStr);
        item5->setFont(QFont(QStringLiteral("monospace"), 9));
        matrixTable_->setItem(r, 5, item5);

        // Column 6: Action Buttons
        auto* actionContainer = new QWidget(matrixTable_);
        auto* actionLayout = new QHBoxLayout(actionContainer);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(4);

        auto* copyHexBtn = new QPushButton(tr("Copy Hex"), actionContainer);
        copyHexBtn->setFixedHeight(22);
        connect(copyHexBtn, &QPushButton::clicked, this, [this, r]() { onCopyHexClicked(r); });
        actionLayout->addWidget(copyHexBtn);

        auto* copyWordsBtn = new QPushButton(tr("Copy Words"), actionContainer);
        copyWordsBtn->setFixedHeight(22);
        connect(copyWordsBtn, &QPushButton::clicked, this, [this, r]() { onCopyWordsClicked(r); });
        actionLayout->addWidget(copyWordsBtn);

        matrixTable_->setCellWidget(r, 6, actionContainer);
    }
}

void Ieee754ConverterWidget::updateBitfieldBreakdown()
{
    if (!binaryFullLabel_ || !signBitLabel_ || !expBitsLabel_ || !fracBitsLabel_ || !formulaLabel_) {
        return;
    }

    if (precision_ == Precision::Float32) {
        const uint32_t u32 = static_cast<uint32_t>(rawData_);
        const uint32_t sign = (u32 >> 31) & 0x01;
        const uint32_t exp = (u32 >> 23) & 0xFF;
        const uint32_t frac = u32 & 0x7FFFFF;

        // Binary string formatted
        QString binSign = QString::number(sign, 2);
        QString binExp = QString::number(exp, 2).rightJustified(8, QLatin1Char('0'));
        QString binFrac = QString::number(frac, 2).rightJustified(23, QLatin1Char('0'));

        binaryFullLabel_->setText(QStringLiteral("<span style='color: #3B82F6;'>%1</span> "
                                                 "<span style='color: #10B981;'>%2</span> "
                                                 "<span style='color: #F59E0B;'>%3</span>")
                                      .arg(binSign, binExp, binFrac));

        // Sign details
        signBitLabel_->setText(tr("Sign [Bit 31]: %1 (%2)").arg(sign).arg(sign == 0 ? tr("Positive +") : tr("Negative -")));

        // Exponent details
        if (exp == 0xFF) {
            expBitsLabel_->setText(tr("Exp [Bits 30-23]: 0xFF (Special: %1)")
                                       .arg(frac == 0 ? QStringLiteral("Infinity") : QStringLiteral("NaN")));
        } else if (exp == 0) {
            expBitsLabel_->setText(tr("Exp [Bits 30-23]: 0 (Subnormal, 2^-126)"));
        } else {
            const int actualExp = static_cast<int>(exp) - 127;
            expBitsLabel_->setText(tr("Exp [Bits 30-23]: %1 (Actual: %1 - 127 = %2, 2^%2)")
                                       .arg(exp).arg(actualExp));
        }

        // Mantissa details
        double fracVal = 0.0;
        for (int i = 0; i < 23; ++i) {
            if ((frac >> (22 - i)) & 1) {
                fracVal += std::pow(2.0, -(i + 1));
            }
        }
        const double significand = (exp == 0) ? fracVal : (1.0 + fracVal);
        fracBitsLabel_->setText(tr("Mantissa [Bits 22-0]: %1 (Significand: %2)")
                                    .arg(QString::number(frac, 16).toUpper().rightJustified(6, QLatin1Char('0')))
                                    .arg(significand, 0, 'g', 8));

        // Evaluation formula
        float fVal = 0.0f;
        std::memcpy(&fVal, &rawData_, 4);
        formulaLabel_->setText(tr("Formula: (-1)^%1 × 2^(%2 - 127) × (1 + 0x%3/2^23) = %4")
                                   .arg(sign)
                                   .arg(exp)
                                   .arg(QString::number(frac, 16).toUpper())
                                   .arg(formatDouble(fVal, 7)));
    } else {
        const uint64_t u64 = rawData_;
        const uint64_t sign = (u64 >> 63) & 0x01;
        const uint64_t exp = (u64 >> 52) & 0x7FF;
        const uint64_t frac = u64 & 0x000FFFFFFFFFFFFFULL;

        QString binSign = QString::number(sign, 2);
        QString binExp = QString::number(exp, 2).rightJustified(11, QLatin1Char('0'));
        QString binFrac = QString::number(frac, 2).rightJustified(52, QLatin1Char('0'));

        binaryFullLabel_->setText(QStringLiteral("<span style='color: #3B82F6;'>%1</span> "
                                                 "<span style='color: #10B981;'>%2</span> "
                                                 "<span style='color: #F59E0B;'>%3</span>")
                                      .arg(binSign, binExp, binFrac));

        signBitLabel_->setText(tr("Sign [Bit 63]: %1 (%2)").arg(sign).arg(sign == 0 ? tr("Positive +") : tr("Negative -")));

        if (exp == 0x7FF) {
            expBitsLabel_->setText(tr("Exp [Bits 62-52]: 0x7FF (Special: %1)")
                                       .arg(frac == 0 ? QStringLiteral("Infinity") : QStringLiteral("NaN")));
        } else if (exp == 0) {
            expBitsLabel_->setText(tr("Exp [Bits 62-52]: 0 (Subnormal, 2^-1022)"));
        } else {
            const int actualExp = static_cast<int>(exp) - 1023;
            expBitsLabel_->setText(tr("Exp [Bits 62-52]: %1 (Actual: %1 - 1023 = %2, 2^%2)")
                                       .arg(exp).arg(actualExp));
        }

        double fracVal = 0.0;
        for (int i = 0; i < 52; ++i) {
            if ((frac >> (51 - i)) & 1) {
                fracVal += std::pow(2.0, -(i + 1));
            }
        }
        const double significand = (exp == 0) ? fracVal : (1.0 + fracVal);
        fracBitsLabel_->setText(tr("Mantissa [Bits 51-0]: %1 (Significand: %2)")
                                    .arg(QString::number(frac, 16).toUpper().rightJustified(13, QLatin1Char('0')))
                                    .arg(significand, 0, 'g', 15));

        double dVal = 0.0;
        std::memcpy(&dVal, &rawData_, 8);
        formulaLabel_->setText(tr("Formula: (-1)^%1 × 2^(%2 - 1023) × (1 + Frac) = %3")
                                   .arg(sign)
                                   .arg(exp)
                                   .arg(formatDouble(dVal, 15)));
    }
}

void Ieee754ConverterWidget::onCopyHexClicked(int row)
{
    if (!matrixTable_ || row < 0 || row >= matrixTable_->rowCount()) return;
    const auto* item = matrixTable_->item(row, 1);
    if (item) {
        copyToClipboard(item->text(), tr("Copied Hex: %1").arg(item->text()));
    }
}

void Ieee754ConverterWidget::onCopyWordsClicked(int row)
{
    if (!matrixTable_ || row < 0 || row >= matrixTable_->rowCount()) return;
    const auto* item = matrixTable_->item(row, 2);
    if (item) {
        copyToClipboard(item->text(), tr("Copied Register Words: %1").arg(item->text()));
    }
}

void Ieee754ConverterWidget::copyToClipboard(const QString& text, const QString& successMsg)
{
    auto* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
        QToolTip::showText(QCursor::pos(), successMsg, this, {}, 2000);
    }
}

QString Ieee754ConverterWidget::formatBinaryWithSpaces(uint64_t val, int bitCount) const
{
    QString s = QString::number(val, 2).rightJustified(bitCount, QLatin1Char('0'));
    QString result;
    for (int i = 0; i < s.length(); ++i) {
        if (i > 0 && i % 4 == 0) result.append(QLatin1Char(' '));
        result.append(s[i]);
    }
    return result;
}

QString Ieee754ConverterWidget::rawHexFormatted(const uint8_t* bytes, int count) const
{
    QStringList parts;
    for (int i = 0; i < count; ++i) {
        parts << QString::number(bytes[i], 16).toUpper().rightJustified(2, QLatin1Char('0'));
    }
    return parts.join(QLatin1Char(' '));
}

} // namespace ui::views::converter
