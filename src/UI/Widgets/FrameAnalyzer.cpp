#include "FrameAnalyzer.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <iomanip>
#include <sstream>

FrameAnalyzer::FrameAnalyzer(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    hexBrowser_ = new QTextBrowser(splitter);
    hexBrowser_->setFont(QFont("Consolas"));
    
    treeWidget_ = new QTreeWidget(splitter);
    treeWidget_->setHeaderLabels({"Field", "Value", "Description"});
    
    detailsBrowser_ = new QTextBrowser(splitter);
    
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 1);
    
    layout->addWidget(splitter);
}

void FrameAnalyzer::analyze(const RawFrame& frame) {
    hexBrowser_->clear();
    treeWidget_->clear();
    detailsBrowser_->clear();
    
    // 1. Hex View
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (uint8_t b : frame.data) {
        ss << std::setw(2) << static_cast<int>(b) << " ";
    }
    hexBrowser_->setText(QString::fromStdString(ss.str()));
    
    // 2. Tree View (Simplified MBAP)
    if (frame.data.size() >= 7) {
        // MBAP
        QTreeWidgetItem* mbap = new QTreeWidgetItem(treeWidget_);
        mbap->setText(0, "MBAP Header");
        
        uint16_t tid = (frame.data[0] << 8) | frame.data[1];
        uint16_t pid = (frame.data[2] << 8) | frame.data[3];
        uint16_t len = (frame.data[4] << 8) | frame.data[5];
        uint8_t uid = frame.data[6];
        
        new QTreeWidgetItem(mbap, {"Transaction ID", QString("0x%1").arg(tid, 4, 16, QChar('0')), "Transaction Identifier"});
        new QTreeWidgetItem(mbap, {"Protocol ID", QString("0x%1").arg(pid, 4, 16, QChar('0')), "0 = Modbus Protocol"});
        new QTreeWidgetItem(mbap, {"Length", QString::number(len), "Number of following bytes"});
        new QTreeWidgetItem(mbap, {"Unit ID", QString::number(uid), "Slave Address"});
        
        // PDU
        QTreeWidgetItem* pdu = new QTreeWidgetItem(treeWidget_);
        pdu->setText(0, "PDU");
        
        if (frame.data.size() > 7) {
            uint8_t fc = frame.data[7];
            new QTreeWidgetItem(pdu, {"Function Code", QString("0x%1").arg(fc, 2, 16, QChar('0')), "Modbus Function"});
        }
        
        treeWidget_->expandAll();
    }
}
