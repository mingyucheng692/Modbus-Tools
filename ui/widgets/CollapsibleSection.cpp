#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QEvent>

namespace ui::widgets {

CollapsibleSection::CollapsibleSection(ui::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    headerWidget_ = new QWidget(this);
    auto headerLayout = new QHBoxLayout(headerWidget_);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    titleLabel_ = new QLabel(headerWidget_);
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();

    toggleButton_ = new QPushButton(headerWidget_);
    toggleButton_->setFixedWidth(28);
    headerLayout->addWidget(toggleButton_);

    bodyGroup_ = new QGroupBox(this);
    bodyGroup_->setTitle(QString());
    auto bodyLayout = new QVBoxLayout(bodyGroup_);
    bodyLayout->setContentsMargins(8, 8, 8, 8);
    contentContainer_ = new QWidget(bodyGroup_);
    bodyLayout->addWidget(contentContainer_);

    mainLayout->addWidget(headerWidget_);
    mainLayout->addWidget(bodyGroup_);

    connect(toggleButton_, &QPushButton::clicked, this, [this]() {
        setExpanded(!expanded_);
    });
    connect(this, &CollapsibleSection::expandedChanged, this, [this]() {
        saveSettings();
    });

    updateToggleUi();
    updateSectionGeometry();
}

QWidget* CollapsibleSection::contentWidget() const {
    return contentContainer_;
}

void CollapsibleSection::setTitle(const QString& title) {
    if (titleLabel_) {
        titleLabel_->setText(title);
    }
}

void CollapsibleSection::setExpanded(bool expanded) {
    if (expanded_ == expanded) {
        return;
    }
    expanded_ = expanded;
    if (bodyGroup_) {
        bodyGroup_->setVisible(expanded_);
    }
    updateToggleUi();
    updateSectionGeometry();
    emit expandedChanged(expanded_);
}

bool CollapsibleSection::isExpanded() const {
    return expanded_;
}

void CollapsibleSection::setSettingsKey(const QString& key) {
    settingsKey_ = key;
    loadSettings();
}

void CollapsibleSection::updateToggleUi() {
    if (!toggleButton_) {
        return;
    }
    toggleButton_->setText(expanded_ ? QStringLiteral("−") : QStringLiteral("⊞"));
    toggleButton_->setToolTip(expanded_ ? tr("Collapse") : tr("Expand"));
}

void CollapsibleSection::updateSectionGeometry() {
    if (!bodyGroup_) {
        return;
    }
    if (expanded_) {
        bodyGroup_->setMaximumHeight(QWIDGETSIZE_MAX);
        setMinimumHeight(0);
        setMaximumHeight(QWIDGETSIZE_MAX);
        QSizePolicy policy = sizePolicy();
        policy.setVerticalPolicy(QSizePolicy::Preferred);
        setSizePolicy(policy);
    } else {
        bodyGroup_->setMaximumHeight(0);
        const int collapsedHeight = headerWidget_ ? headerWidget_->sizeHint().height() : 0;
        setMinimumHeight(collapsedHeight);
        setMaximumHeight(collapsedHeight);
        QSizePolicy policy = sizePolicy();
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        setSizePolicy(policy);
    }
    updateGeometry();
    if (auto* parent = parentWidget()) {
        parent->updateGeometry();
    }
}

void CollapsibleSection::loadSettings() {
    if (settingsKey_.isEmpty()) {
        if (bodyGroup_) {
            bodyGroup_->setVisible(expanded_);
        }
        updateToggleUi();
        updateSectionGeometry();
        return;
    }
    const bool collapsed = settingsService_ ? settingsService_->value(settingsKey_).toBool() : false;
    expanded_ = !collapsed;
    if (bodyGroup_) {
        bodyGroup_->setVisible(expanded_);
    }
    updateToggleUi();
    updateSectionGeometry();
    if (settingsService_ && !settingsService_->contains(settingsKey_)) {
        settingsService_->setValue(settingsKey_, false);
    }
}

void CollapsibleSection::saveSettings() const {
    if (settingsKey_.isEmpty() || !settingsService_) {
        return;
    }
    settingsService_->setValue(settingsKey_, !expanded_);
}

void CollapsibleSection::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        updateToggleUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
