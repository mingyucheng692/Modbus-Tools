#include "CollapsibleSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSettings>
#include <QApplication>
#include <QEvent>

namespace ui::widgets {

CollapsibleSection::CollapsibleSection(QWidget* parent)
    : QWidget(parent) {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerFrame_ = new QFrame(this);
    headerFrame_->setFrameShape(QFrame::StyledPanel);
    auto headerLayout = new QHBoxLayout(headerFrame_);
    headerLayout->setContentsMargins(8, 4, 8, 4);

    titleLabel_ = new QLabel(headerFrame_);
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();

    toggleButton_ = new QPushButton(headerFrame_);
    toggleButton_->setFixedWidth(28);
    headerLayout->addWidget(toggleButton_);

    contentContainer_ = new QWidget(this);

    mainLayout->addWidget(headerFrame_);
    mainLayout->addWidget(contentContainer_);

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
    if (contentContainer_) {
        contentContainer_->setVisible(expanded_);
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
        if (contentContainer_) {
            contentContainer_->setVisible(expanded_);
        }
        updateToggleUi();
        return;
    }
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    const bool collapsed = settings.value(settingsKey_, false).toBool();
    expanded_ = !collapsed;
    if (contentContainer_) {
        contentContainer_->setVisible(expanded_);
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
