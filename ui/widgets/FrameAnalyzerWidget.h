#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QList>
#include "modbus/parser/ModbusFrameParser.h"

class QPlainTextEdit;
class QComboBox;
class QSpinBox;
class QPushButton;
class QTableWidget;
class QLabel;
class QTreeWidget;
class QTabWidget;
class QGroupBox;
class QTableWidgetItem;
class QWidget;
class QListWidget;
class QString;
class QThread;
class QObject;

namespace ui::widgets {

class FrameAnalyzerWidget : public QWidget {
    Q_OBJECT

public:
    explicit FrameAnalyzerWidget(QWidget* parent = nullptr);
    ~FrameAnalyzerWidget() override;

signals:
    void parseRequested(const QString& input, modbus::core::parser::ProtocolType type, uint16_t startAddress, quint64 requestId);

private slots:
    void onParseClicked();
    void onClearClicked();
    void onFormatClicked(); // 格式化 Hex 输入
    void onExportJsonClicked();
    void onImportJsonClicked();
    void onExportCsvClicked();
    void onCopyRegisterMapClicked();
    void onHistorySelectionChanged(int row);
    void onClearHistoryClicked();
    void onParseFinished(const modbus::core::parser::ParseResult& result, quint64 requestId);

protected:
    void changeEvent(QEvent* event) override;

private:
    enum class NumberDisplayMode {
        Unsigned,
        Signed
    };
    struct DataMetadata {
        double scale = 1.0;
        QString description;
    };

    void setupUi();
    void retranslateUi();
    void createInputGroup();
    void createResultGroup();
    void loadSettings();
    void saveSettings();
    QString formatDecimalValue(const QVariant& value) const;
    QString formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex) const;
    QString formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary) const;
    void onDataTableItemChanged(QTableWidgetItem* item);
    uint16_t rowAddress(int row) const;
    double numericValueForDisplay(const QVariant& value, bool* ok) const;
    QString formatScaledValue(const QVariant& value, const DataMetadata& meta) const;
    QString buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta) const;
    void applyMetadataToRow(int row, const QVariant& value, const DataMetadata& meta);
    void exportMetadataToJson(const QString& filePath);
    void importMetadataFromJson(const QString& filePath);
    QString formatRawFrameStructure(const modbus::core::parser::ParseResult& result) const;
    void exportCurrentTableToCsv(const QString& filePath) const;
    QString buildRegisterMapText() const;
    QString escapeCsvValue(const QString& value) const;
    void addToHistory(const modbus::core::parser::ParseResult& result);
    void refreshHistoryList();
    QString historyItemText(const modbus::core::parser::ParseResult& result) const;
    
    // 渲染解析结果
    void renderResult(const modbus::core::parser::ParseResult& result);
    void clearResult();

private:
    // Input Controls
    QGroupBox* inputGroup_ = nullptr;
    QLabel* protocolLabel_ = nullptr;
    QLabel* startAddrLabel_ = nullptr;
    QLabel* displayModeLabel_ = nullptr;
    QPlainTextEdit* inputEditor_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QComboBox* displayModeCombo_ = nullptr;
    QSpinBox* startAddressSpin_ = nullptr;
    QPushButton* parseBtn_ = nullptr;
    QPushButton* formatBtn_ = nullptr;
    QPushButton* importJsonBtn_ = nullptr;
    QPushButton* exportJsonBtn_ = nullptr;
    QPushButton* exportCsvBtn_ = nullptr;
    QPushButton* copyMapBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;

    // Result Controls
    QGroupBox* resultGroup_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QWidget* structureTab_ = nullptr;
    QTreeWidget* overviewTree_ = nullptr;
    QPlainTextEdit* rawFrameView_ = nullptr;
    QTableWidget* dataTable_ = nullptr;
    QTabWidget* resultTabs_ = nullptr;
    QGroupBox* historyGroup_ = nullptr;
    QListWidget* historyList_ = nullptr;
    QPushButton* clearHistoryBtn_ = nullptr;
    NumberDisplayMode displayMode_ = NumberDisplayMode::Unsigned;
    QMap<uint16_t, DataMetadata> metadataByAddress_;
    QList<modbus::core::parser::ParseResult> historyResults_;
    modbus::core::parser::ParseResult currentResult_;
    QThread* parseThread_ = nullptr;
    QObject* parseWorker_ = nullptr;
    quint64 latestParseRequestId_ = 0;
    bool parseInProgress_ = false;
    bool isUpdatingDataTable_ = false;
    static constexpr int kMaxHistoryCount = 20;
};

} // namespace ui::widgets
