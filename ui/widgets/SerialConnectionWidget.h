#pragma once

#include <QWidget>
#include <QSerialPort>
#include "../../../core/io/SerialChannel.h" // For SerialConfig

class QComboBox;
class QPushButton;
class QLabel;

namespace ui::widgets {

class SerialConnectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit SerialConnectionWidget(QWidget *parent = nullptr);
    ~SerialConnectionWidget() override;

    io::SerialConfig getConfig() const;

signals:
    void connectClicked(const io::SerialConfig& config);
    void disconnectClicked();

public slots:
    void setConnected(bool connected);
    void refreshPorts();

private:
    void setupUi();

    QComboBox* portCombo_ = nullptr;
    QComboBox* baudCombo_ = nullptr;
    QComboBox* dataBitsCombo_ = nullptr;
    QComboBox* parityCombo_ = nullptr;
    QComboBox* stopBitsCombo_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    
    bool isConnected_ = false;
};

} // namespace ui::widgets
