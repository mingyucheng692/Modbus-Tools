#pragma once

#include <QWidget>
#include <QByteArray>

class QListWidget;
class QCheckBox;
class QPushButton;
class QComboBox;
class QGroupBox;
class QEvent;

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

private slots:
    void onSaveClicked();
    void onCopyClicked();

private:
    void setupUi();
    QString formatData(const QByteArray& data) const;
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QGroupBox* groupBox_ = nullptr;
    QListWidget* logList_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QCheckBox* showTxCheck_ = nullptr;
    QCheckBox* showRxCheck_ = nullptr;
    QComboBox* displayModeBox_ = nullptr; // Hex, ASCII
    QPushButton* clearBtn_ = nullptr;
    QPushButton* saveBtn_ = nullptr;
};

} // namespace ui::widgets
