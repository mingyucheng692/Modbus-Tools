#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QTextBrowser>
#include "Core/Modbus/RawFrame.h"

class FrameAnalyzer : public QWidget {
    Q_OBJECT
public:
    explicit FrameAnalyzer(QWidget* parent = nullptr);

    void analyze(const RawFrame& frame);

private:
    void analyzePdu(QTreeWidgetItem* parent, uint8_t fc, const std::vector<uint8_t>& payload);

    QTreeWidget* treeWidget_;
    QTextBrowser* detailsBrowser_;
    QTextBrowser* hexBrowser_;
};
