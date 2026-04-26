/**
 * @file FrameAnalyzerWidget.h
 * @brief Header file for FrameAnalyzerWidget.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {

class FrameAnalyzerWidget : public QWidget {
    Q_OBJECT
    class FrameAnalyzerWidgetPrivate;
    Q_DECLARE_PRIVATE(FrameAnalyzerWidget)

public:
    explicit FrameAnalyzerWidget(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~FrameAnalyzerWidget() override;

    /**
     * @brief 处理来自外部（如会话视图）的实时 PDU 数据。
     */
    void processLivePdu(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);
    void exitLiveMode();

signals:
    void linkageStopRequested();

private slots:
    void onParseClicked();
    void onClearClicked();
    void onFormatClicked();
    void onExportJsonClicked();
    void onImportJsonClicked();
    void onExportCsvClicked();
    void onHistorySelectionChanged(int row);
    void onClearHistoryClicked();

    // 内部槽函数，由 Private 类进行跨线程桥接
    void onParseFinished(const modbus::core::parser::ParseResult& result, quint64 requestId);

protected:
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    
    // 渲染解析结果
    void renderResult(const modbus::core::parser::ParseResult& result);
    void clearResult();

    QScopedPointer<FrameAnalyzerWidgetPrivate> d_ptr;
};

} // namespace ui::widgets
