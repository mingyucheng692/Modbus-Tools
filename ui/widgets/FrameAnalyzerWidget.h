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

private:
    void setupUi();
    void createInputGroup();
    void createResultGroup();
    
    // 渲染解析结果
    void renderResult(const struct modbus::core::parser::ParseResult& result);
    void clearResult();

private:
    // Input Controls
    QPlainTextEdit* inputEditor_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QSpinBox* startAddressSpin_ = nullptr;
    QPushButton* parseBtn_ = nullptr;
    QPushButton* formatBtn_ = nullptr;

    // Result Controls
    QLabel* statusLabel_ = nullptr;
    QTreeWidget* overviewTree_ = nullptr;
    QTableWidget* dataTable_ = nullptr;
    QTabWidget* resultTabs_ = nullptr;
};

} // namespace ui::widgets
