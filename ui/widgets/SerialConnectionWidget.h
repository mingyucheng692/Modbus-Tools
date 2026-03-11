#pragma once

#include <QWidget>
#include <QSerialPort>
#include "../../../core/io/SerialChannel.h" // For SerialConfig

class QComboBox;
class QPushButton;
class QLabel;
class QEvent;
class QString;

namespace ui::widgets {
class CollapsibleSection;

class SerialConnectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit SerialConnectionWidget(QWidget *parent = nullptr);
    ~SerialConnectionWidget() override;

    io::SerialConfig getConfig() const;
    void setSettingsGroup(const QString& group);

signals:
    void connectClicked(const io::SerialConfig& config);
    void disconnectClicked();

public slots:
    void setConnected(bool connected);
    void refreshPorts();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QLabel* portLabel_ = nullptr;
    QComboBox* portCombo_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    QLabel* baudLabel_ = nullptr;
    QComboBox* baudCombo_ = nullptr;
    QLabel* dataBitsLabel_ = nullptr;
    QComboBox* dataBitsCombo_ = nullptr;
    QLabel* parityLabel_ = nullptr;
    QComboBox* parityCombo_ = nullptr;
    QLabel* stopBitsLabel_ = nullptr;
    QComboBox* stopBitsCombo_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    CollapsibleSection* section_ = nullptr;
    
    bool isConnected_ = false;
    QString settingsGroup_ = "serial";
};

} // namespace ui::widgets
