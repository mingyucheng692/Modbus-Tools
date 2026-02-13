#include "RtuView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui::views::rtu {

RtuView::RtuView(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

RtuView::~RtuView() = default;

void RtuView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Placeholder content
    titleLabel_ = new QLabel("Modbus RTU Debugger", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    QFont font = titleLabel_->font();
    font.setPointSize(16);
    font.setBold(true);
    titleLabel_->setFont(font);
    
    mainLayout_->addWidget(titleLabel_);
    mainLayout_->addStretch();
}

} // namespace ui::views::rtu
