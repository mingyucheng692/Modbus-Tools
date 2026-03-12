#pragma once

#include <QWidget>
#include <QScopedPointer>

class QPlainTextEdit;
class QComboBox;
class QSpinBox;
class QPushButton;
class QTableWidget;
class QLabel;
class QTreeWidget;
class QTabWidget;
class QGroupBox;
namespace modbus::core::parser {
    struct ParseResult;
}

namespace ui::widgets {

class FrameAnalyzerWidget : public QWidget {
    Q_OBJECT

public:
    explicit FrameAnalyzerWidget(QWidget* parent = nullptr);
    ~FrameAnalyzerWidget() override;

private slots:
    void onParseClicked();
    void onClearClicked();
    void onFormatClicked(); // 格式化 Hex 输入

protected:
    void changeEvent(QEvent* event) override;

private:
    enum class NumberDisplayMode {
        Unsigned,
        Signed
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
    
    // 渲染解析结果
    void renderResult(const struct modbus::core::parser::ParseResult& result);
    void clearResult();

private:
    // Input Controls
    QGroupBox* inputGroup_ = nullptr;
    QLabel* protocolLabel_ = nullptr;
    QLabel* startAddrLabel_ = nullptr;
    QPlainTextEdit* inputEditor_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QSpinBox* startAddressSpin_ = nullptr;
    QPushButton* parseBtn_ = nullptr;
    QPushButton* formatBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;

    // Result Controls
    QGroupBox* resultGroup_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QTreeWidget* overviewTree_ = nullptr;
    QTableWidget* dataTable_ = nullptr;
    QTabWidget* resultTabs_ = nullptr;
    NumberDisplayMode displayMode_ = NumberDisplayMode::Unsigned;
};

} // namespace ui::widgets
