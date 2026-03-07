#pragma once

#include <QWidget>
#include <QTimer>

class QTextEdit;
class QRadioButton;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QProgressBar;
class QEvent;
class QString;

namespace ui::widgets {

class GenericInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit GenericInputWidget(QWidget *parent = nullptr);
    ~GenericInputWidget() override;

    void appendData(const QByteArray& data); // For feedback if needed? No, input is for sending.
    void setSettingsGroup(const QString& group);

signals:
    void sendRequested(const QByteArray& data);
    void fileSendRequested(const QString& filePath); // Or handle internally?
    // Handling large file send internally or via worker is better.
    // But UI just requests it.
    
    // Actually, GenericIoWorker handles writing.
    // If we send a large file, we might want to chunk it.
    // For now, let's just load file and send if small, or stream if large.
    // Let's expose a signal to send data.
    
private slots:
    void onSendClicked();
    void onSendFileClicked();
    void onAutoSendToggled(bool checked);
    void onTimerTimeout();
    void onFormatChanged();

private:
    void setupUi();
    QByteArray getData() const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QTextEdit* inputEdit_ = nullptr;
    QRadioButton* hexRadio_ = nullptr;
    QRadioButton* asciiRadio_ = nullptr;
    QCheckBox* autoSendCheck_ = nullptr;
    QSpinBox* intervalSpin_ = nullptr;
    QPushButton* sendBtn_ = nullptr;
    QPushButton* sendFileBtn_ = nullptr;
    
    QTimer* autoSendTimer_ = nullptr;
    QString settingsGroup_;
};

} // namespace ui::widgets
