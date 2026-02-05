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
    QTreeWidget* treeWidget_;
    QTextBrowser* detailsBrowser_;
    QTextBrowser* hexBrowser_;
};
