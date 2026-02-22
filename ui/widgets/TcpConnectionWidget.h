#pragma once

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;

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

    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    
    bool isConnected_ = false;
};

} // namespace ui::widgets
