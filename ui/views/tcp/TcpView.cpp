#include "TcpView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui::views::tcp {

TcpView::TcpView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

TcpView::~TcpView() = default;

void TcpView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Placeholder content
    titleLabel_ = new QLabel("Modbus TCP Debugger", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    QFont font = titleLabel_->font();
    font.setPointSize(16);
    font.setBold(true);
    titleLabel_->setFont(font);
    
    mainLayout_->addWidget(titleLabel_);
    mainLayout_->addStretch();
}

} // namespace ui::views::tcp
