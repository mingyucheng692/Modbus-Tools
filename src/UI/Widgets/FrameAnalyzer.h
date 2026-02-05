#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QTextBrowser>
#include <QComboBox>
#include "Core/Modbus/RawFrame.h"
#include "Core/Modbus/EndianUtils.h"

class FrameAnalyzer : public QWidget {
    Q_OBJECT
public:
    explicit FrameAnalyzer(QWidget* parent = nullptr);

    void analyze(const RawFrame& frame);

private slots:
    void onEndianChanged(int index);

private:
    void analyzePdu(QTreeWidgetItem* parent, uint8_t fc, const std::vector<uint8_t>& payload);

    QTreeWidget* treeWidget_;
    QTextBrowser* detailsBrowser_;
    QTextBrowser* hexBrowser_;
    QComboBox* endianCombo_;
    
    RawFrame lastFrame_;
    Modbus::Endianness currentEndian_ = Modbus::Endianness::ABCD;
};
