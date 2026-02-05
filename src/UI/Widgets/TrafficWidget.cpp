#include "TrafficWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include "Core/Logging/TrafficLog.h"

TrafficWidget::TrafficWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    
    // Toolbar
    QHBoxLayout* toolLayout = new QHBoxLayout();
    clearBtn_ = new QPushButton("Clear", this);
    exportBtn_ = new QPushButton("Export CSV", this);
    autoScrollCheck_ = new QCheckBox("Auto Scroll", this);
    autoScrollCheck_->setChecked(true);
    
    toolLayout->addWidget(clearBtn_);
    toolLayout->addWidget(exportBtn_);
    toolLayout->addWidget(autoScrollCheck_);
    toolLayout->addStretch();
    
    layout->addLayout(toolLayout);
    
    listView_ = new QListView(this);
    model_ = new TrafficModel(this);
    listView_->setModel(model_);
    listView_->setUniformItemSizes(true);
    listView_->setAutoScroll(true);
    
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    listView_->setFont(font);

    layout->addWidget(listView_);
    
    connect(listView_->selectionModel(), &QItemSelectionModel::currentChanged, 
        [this](const QModelIndex& current, const QModelIndex&){
            if (current.isValid()) emit frameSelected(current.row());
        });
        
    connect(clearBtn_, &QPushButton::clicked, this, &TrafficWidget::onClearClicked);
    connect(exportBtn_, &QPushButton::clicked, this, &TrafficWidget::onExportClicked);
    connect(model_, &QAbstractListModel::rowsInserted, [this](){
        if (autoScrollCheck_->isChecked()) {
            listView_->scrollToBottom();
        }
    });
}

void TrafficWidget::onClearClicked() {
    TrafficLog::instance().clear();
}

void TrafficWidget::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Traffic Log", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot write to file");
        return;
    }
    
    QTextStream out(&file);
    out << "Timestamp,Direction,Hex,ASCII\n";
    
    size_t count = TrafficLog::instance().count();
    for (size_t i = 0; i < count; ++i) {
        const auto& frame = TrafficLog::instance().getFrame(i);
        
        // Timestamp
        auto time = std::chrono::system_clock::to_time_t(frame.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(frame.timestamp.time_since_epoch()) % 1000;
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        
        out << buf << "." << ms.count() << ",";
        out << (frame.direction == Direction::Tx ? "TX" : "RX") << ",";
        
        // Hex
        QString hex;
        for (uint8_t b : frame.data) hex += QString("%1 ").arg(b, 2, 16, QChar('0')).toUpper();
        out << hex.trimmed() << ",";
        
        // ASCII
        QString ascii;
        for (uint8_t b : frame.data) {
            if (b >= 32 && b <= 126) ascii += static_cast<char>(b);
            else ascii += ".";
        }
        out << ascii << "\n";
    }
}
