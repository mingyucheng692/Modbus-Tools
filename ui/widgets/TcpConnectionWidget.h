#pragma once

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QGroupBox;
class QEvent;

namespace ui::widgets {

class TcpConnectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit TcpConnectionWidget(QWidget *parent = nullptr);
    ~TcpConnectionWidget() override;

    void setDefaultPort(int port);
    QString getIpAddress() const;
    int getPort() const;

signals:
    void connectClicked(const QString& ip, int port);
    void disconnectClicked();

public slots:
    void setConnected(bool connected);

private:
    void setupUi();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QGroupBox* groupBox_ = nullptr;
    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    
    bool isConnected_ = false;
};

} // namespace ui::widgets
