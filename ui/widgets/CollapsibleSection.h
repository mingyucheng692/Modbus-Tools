#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QFrame;
class QEvent;
class QString;

namespace ui::widgets {

class CollapsibleSection : public QWidget {
    Q_OBJECT

public:
    explicit CollapsibleSection(QWidget* parent = nullptr);
    QWidget* contentWidget() const;
    void setTitle(const QString& title);
    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setSettingsKey(const QString& key);

signals:
    void expandedChanged(bool expanded);

private:
    void updateToggleUi();
    void loadSettings();
    void saveSettings() const;
    void changeEvent(QEvent* event) override;

    QFrame* headerFrame_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QPushButton* toggleButton_ = nullptr;
    QWidget* contentContainer_ = nullptr;
    QString settingsKey_;
    bool expanded_ = true;
};

} // namespace ui::widgets
