#pragma once

#include <QWidget>
#include <QByteArray>

class QListWidget;
class QCheckBox;
class QPushButton;
class QGroupBox;
class QEvent;
class QString;

namespace ui::widgets {

class TrafficMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrafficMonitorWidget(QWidget *parent = nullptr);
    ~TrafficMonitorWidget() override;

    void appendTx(const QByteArray& data);
    void appendRx(const QByteArray& data);
    void appendInfo(const QString& message);
    void clear();
    void setSettingsGroup(const QString& group);

private slots:
    void onSaveClicked();
    void onCopyClicked();

private:
    void setupUi();
    QString formatData(const QByteArray& data) const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QGroupBox* groupBox_ = nullptr;
    QListWidget* logList_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QCheckBox* showTxCheck_ = nullptr;
    QCheckBox* showRxCheck_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* saveBtn_ = nullptr;

    QString settingsGroup_;
};

} // namespace ui::widgets
