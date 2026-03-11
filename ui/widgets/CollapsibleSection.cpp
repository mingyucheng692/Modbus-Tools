#include "CollapsibleSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QSettings>
#include <QApplication>
#include <QEvent>

namespace ui::widgets {

CollapsibleSection::CollapsibleSection(QWidget* parent)
    : QWidget(parent) {
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

void CollapsibleSection::loadSettings() {
    if (settingsKey_.isEmpty()) {
        if (bodyGroup_) {
            bodyGroup_->setVisible(expanded_);
        }
        updateToggleUi();
        return;
    }
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    const bool collapsed = settings.value(settingsKey_, false).toBool();
    expanded_ = !collapsed;
    if (bodyGroup_) {
        bodyGroup_->setVisible(expanded_);
    }
    updateToggleUi();
    if (!settings.contains(settingsKey_)) {
        settings.setValue(settingsKey_, false);
    }
}

void CollapsibleSection::saveSettings() const {
    if (settingsKey_.isEmpty()) {
        return;
    }
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue(settingsKey_, !expanded_);
}

void CollapsibleSection::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        updateToggleUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
