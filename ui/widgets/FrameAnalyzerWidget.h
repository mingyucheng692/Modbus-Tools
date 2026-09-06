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
#include <QList>
#include <QMap>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "analyzer/AnalyzerCommon.h"

class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QSplitter;
class QTabWidget;
class QTableWidget;
class QTreeWidget;

namespace infra::config {
class ISettingsService;
}

namespace ui::application::analyzer {
class FrameAnalyzerPresenter;
}

namespace ui::widgets {

class FrameDecodedTableView;

/**
 * @brief Modbus frame analyzer view: rendering and interaction only.
 *
 * The background parse QThread + FrameParseWorker pair and its teardown are
 * owned by ui::application::analyzer::FrameAnalyzerPresenter (created as a
 * child of this widget). No PIMPL: the application has no ABI-stability
 * requirement, members are declared directly.
 */
class FrameAnalyzerWidget : public QWidget {
    Q_OBJECT

public:
    explicit FrameAnalyzerWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~FrameAnalyzerWidget() override;

    /**
     * @brief 处理来自外部（如会话视图）的实时 PDU 数据。
     */
    void processLivePdu(const modbus::base::Pdu& pdu, modbus::parser::ProtocolType protocol, uint16_t addr);
    void exitLiveMode();
    void setLivePaused(bool paused);

signals:
    void linkagePauseToggled(bool paused);
    void linkageStopRequested();

private slots:
    void onParseClicked();
    void onPasteAndParseClicked();
    void onClearClicked();
    void onFormatClicked();
    void onExportJsonClicked();
    void onImportJsonClicked();
    void onExportCsvClicked();
    void onHistorySelectionChanged(int row);
    void onClearHistoryClicked();
    void onResetTypesClicked();

    // 由 FrameAnalyzerPresenter 在 GUI 线程上转发
    void onParseFinished(const modbus::parser::ParseResult& result, quint64 requestId);

protected:
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    void renderResult(const modbus::parser::ParseResult& result);
    void clearResult();

    // --- Helpers ---
    [[nodiscard]] QString historyItemText(const modbus::parser::ParseResult& result) const;
    void addToHistory(const modbus::parser::ParseResult& result);
    void refreshHistoryList();
    void setHistoryCollapsed(bool collapsed);
    void updateHistoryToggleText();
    void updateAdaptiveLayout();
    void loadSettings();
    void saveSettings();
    void updateResetButtonState(int selectedCount, bool hasCustom);
    void updateResetButtonState();

    // --- UI construction ---
    void createInputGroup();
    void createResultGroup();

    // --- Services / threading ---
    infra::config::ISettingsService* settingsService_ = nullptr;
    ui::application::analyzer::FrameAnalyzerPresenter* presenter_ = nullptr;

    // --- Input controls ---
    QGroupBox* inputGroup = nullptr;
    QSplitter* mainSplitter = nullptr;
    QLabel* protocolLabel = nullptr;
    QLabel* startAddrLabel = nullptr;
    QLabel* displayModeLabel = nullptr;
    QPlainTextEdit* inputEditor = nullptr;
    QComboBox* protocolCombo = nullptr;
    QComboBox* displayModeCombo = nullptr;
    QComboBox* registerOrderCombo = nullptr;
    QPushButton* parseBtn = nullptr;
    QPushButton* pasteAndParseBtn = nullptr;
    QPushButton* formatBtn = nullptr;
    QPushButton* importJsonBtn = nullptr;
    QPushButton* exportJsonBtn = nullptr;
    QPushButton* exportCsvBtn = nullptr;
    QPushButton* toggleHistoryBtn = nullptr;
    QPushButton* clearBtn = nullptr;
    QPushButton* resetTypesBtn = nullptr;
    QLabel* registerOrderLabel = nullptr;
    QLineEdit* startAddrEdit = nullptr;

    // --- Result controls ---
    QGroupBox* resultGroup = nullptr;
    QLabel* statusTitleLabel = nullptr;
    QLabel* statusLabel = nullptr;
    QWidget* structureTab = nullptr;
    QTreeWidget* overviewTree = nullptr;
    FrameDecodedTableView* dataTable = nullptr;
    QTabWidget* resultTabs = nullptr;
    QSplitter* contentSplitter = nullptr;
    QGroupBox* historyGroup = nullptr;
    QListWidget* historyList = nullptr;
    QPushButton* clearHistoryBtn = nullptr;

    // --- Live link UI ---
    QLabel* liveLabel = nullptr;
    QLabel* linkageTipLabel = nullptr;
    QPushButton* linkagePauseBtn = nullptr;
    QPushButton* linkageStopBtn = nullptr;

    // --- State ---
    bool historyCollapsed = false;
    bool historyAutoCollapsed = false;
    int lastHistoryPanelWidth = 0; // set in the constructor from config::Ui
    modbus::analyzer::RegisterDataType globalDataType = modbus::analyzer::RegisterDataType::UInt16;
    modbus::analyzer::NumberDisplayMode displayMode = modbus::analyzer::NumberDisplayMode::Unsigned;
    QList<modbus::parser::ParseResult> historyResults;
    modbus::parser::ParseResult currentResult;
    quint64 latestParseRequestId = 0;
    bool parseInProgress = false;
    bool isLiveMode = false;
    bool isLivePaused = false;
    modbus::parser::ParseResult lastLiveResult;
    modbus::base::RegisterOrder registerOrder = modbus::base::RegisterOrder::ABCD;
};

} // namespace ui::widgets
