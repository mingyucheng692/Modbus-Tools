#pragma once

#include <QWidget>
#include <cstdint>

class QSpinBox;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QLabel;
class QPushButton;
class QEvent;
class QString;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {
class CollapsibleSection;

class FunctionWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~FunctionWidget() override;

    int getSlaveId() const;
    int getStartAddress() const;
    int getQuantity() const;
    int getFormatIndex() const;
    void setSettingsGroup(const QString& group);

signals:
    // Read: Function Code, Address, Quantity
    void readRequested(uint8_t functionCode, int address, int quantity, int slaveId);
    
    // Write: Function Code, Address, Data, Format, SlaveId
    void writeRequested(uint8_t functionCode, int address, const QString& data, const QString& format, int slaveId);

    // Raw Send: Data
    void rawSendRequested(const QByteArray& data);

private:
    void setupUi();
    void setupStandardUi(QWidget* parent);
    void setupRawUi(QWidget* parent);
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;
    
    void onReadClicked(uint8_t functionCode);
    void onWriteClicked(uint8_t functionCode);
    void onRawSendClicked();
    void onFormatChanged();

    // Standard UI
    CollapsibleSection* standardSection_ = nullptr;
    CollapsibleSection* rawSection_ = nullptr;
    QLabel* slaveIdLabel_ = nullptr;
    QSpinBox* slaveIdEdit_ = nullptr;
    QLabel* addressLabel_ = nullptr;
    QSpinBox* addressEdit_ = nullptr;
    QLabel* quantityLabel_ = nullptr;
    QSpinBox* quantityEdit_ = nullptr;
    QLabel* writeDataLabel_ = nullptr;
    QLineEdit* writeDataEdit_ = nullptr;
    QLabel* formatLabel_ = nullptr;
    QComboBox* dataFormatBox_ = nullptr;
    QPushButton* readBtn01_ = nullptr;
    QPushButton* readBtn02_ = nullptr;
    QPushButton* readBtn03_ = nullptr;
    QPushButton* readBtn04_ = nullptr;
    QPushButton* writeBtn05_ = nullptr;
    QPushButton* writeBtn06_ = nullptr;
    QPushButton* writeBtn0F_ = nullptr;
    QPushButton* writeBtn10_ = nullptr;

    // Raw UI
    QLabel* rawDataLabel_ = nullptr;
    QTextEdit* rawDataEdit_ = nullptr;
    QPushButton* rawSendBtn_ = nullptr;

    QString settingsGroup_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
