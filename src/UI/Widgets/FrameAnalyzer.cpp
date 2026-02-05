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
            
            // Payload Analysis
            if (frame.data.size() > 8) {
                std::vector<uint8_t> payload(frame.data.begin() + 8, frame.data.end());
                analyzePdu(pdu, fc, payload);
            }
        }
        
        treeWidget_->expandAll();
    }
}

void FrameAnalyzer::analyzePdu(QTreeWidgetItem* parent, uint8_t fc, const std::vector<uint8_t>& payload) {
    if (fc & 0x80) { // Exception
        uint8_t ec = payload[0];
        QString desc = "Unknown Error";
        switch(ec) {
            case 0x01: desc = "Illegal Function"; break;
            case 0x02: desc = "Illegal Data Address"; break;
            case 0x03: desc = "Illegal Data Value"; break;
            case 0x04: desc = "Server Device Failure"; break;
        }
        new QTreeWidgetItem(parent, {"Exception Code", QString("0x%1").arg(ec, 2, 16, QChar('0')), desc});
        return;
    }

    switch(fc) {
        case 0x03: // Read Holding Registers Response
        case 0x04: // Read Input Registers Response
        {
            if (payload.size() < 1) return;
            uint8_t byteCount = payload[0];
            new QTreeWidgetItem(parent, {"Byte Count", QString::number(byteCount), ""});
            
            int regCount = byteCount / 2;
            for (int i = 0; i < regCount; ++i) {
                if (1 + i*2 + 1 < payload.size()) {
                    uint16_t val = (payload[1 + i*2] << 8) | payload[1 + i*2 + 1];
                    new QTreeWidgetItem(parent, {
                        QString("Register %1").arg(i), 
                        QString("0x%1").arg(val, 4, 16, QChar('0')), 
                        QString::number(val)
                    });
                }
            }
            break;
        }
        case 0x05: // Write Single Coil Request
        case 0x06: // Write Single Register Request
        {
            if (payload.size() < 4) return;
            uint16_t addr = (payload[0] << 8) | payload[1];
            uint16_t val = (payload[2] << 8) | payload[3];
            
            new QTreeWidgetItem(parent, {"Address", QString("0x%1").arg(addr, 4, 16, QChar('0')), QString::number(addr)});
            new QTreeWidgetItem(parent, {"Value", QString("0x%1").arg(val, 4, 16, QChar('0')), 
                (fc == 5) ? (val == 0xFF00 ? "ON" : "OFF") : QString::number(val)});
            break;
        }
        case 0x10: // Write Multiple Registers Request
        {
            if (payload.size() < 5) return;
            uint16_t addr = (payload[0] << 8) | payload[1];
            uint16_t count = (payload[2] << 8) | payload[3];
            uint8_t bytes = payload[4];
            
            new QTreeWidgetItem(parent, {"Starting Address", QString("0x%1").arg(addr, 4, 16, QChar('0')), QString::number(addr)});
            new QTreeWidgetItem(parent, {"Quantity", QString::number(count), ""});
            new QTreeWidgetItem(parent, {"Byte Count", QString::number(bytes), ""});
            
            int regCount = bytes / 2;
            for (int i = 0; i < regCount; ++i) {
                if (5 + i*2 + 1 < payload.size()) {
                    uint16_t val = (payload[5 + i*2] << 8) | payload[5 + i*2 + 1];
                    new QTreeWidgetItem(parent, {
                        QString("Register %1").arg(i), 
                        QString("0x%1").arg(val, 4, 16, QChar('0')), 
                        QString::number(val)
                    });
                }
            }
            break;
        }
    }
}
