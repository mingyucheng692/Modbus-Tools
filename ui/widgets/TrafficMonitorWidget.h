#pragma once

#include <QWidget>
#include <QByteArray>

class QListWidget;
class QCheckBox;
class QPushButton;
class QComboBox;

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

private:
    void setupUi();
    QString formatData(const QByteArray& data) const;

    QListWidget* logList_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QComboBox* displayModeBox_ = nullptr; // Hex, ASCII
    QPushButton* clearBtn_ = nullptr;
};

} // namespace ui::widgets
