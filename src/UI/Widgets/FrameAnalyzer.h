#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QTextBrowser>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "Core/Modbus/RawFrame.h"
#include "Core/Modbus/EndianUtils.h"

class FrameAnalyzer : public QWidget {
    Q_OBJECT
public:
    explicit FrameAnalyzer(QWidget* parent = nullptr);

    void analyze(const RawFrame& frame);

private slots:
    void onEndianChanged(int index);
    void onAnalyzeClicked();
    void onTreeContextMenu(const QPoint& pos);

signals:
    void addToWaveformRequested(int address);

private:
    void analyzePdu(QTreeWidgetItem* parent, uint8_t fc, const std::vector<uint8_t>& payload);

    QLineEdit* inputEdit_;
    QPushButton* analyzeBtn_;
    QTreeWidget* treeWidget_;
    QTextBrowser* detailsBrowser_;
    QTextBrowser* hexBrowser_;
    QComboBox* endianCombo_;
    
    RawFrame lastFrame_;
    Modbus::Endianness currentEndian_ = Modbus::Endianness::ABCD;
};
