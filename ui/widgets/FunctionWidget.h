#pragma once

#include <QWidget>
#include <cstdint>

class QSpinBox;
class QLineEdit;
class QComboBox;
class QTextEdit;

namespace ui::widgets {

class FunctionWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionWidget(QWidget *parent = nullptr);
    ~FunctionWidget() override;

    int getSlaveId() const;

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
    
    void onReadClicked(uint8_t functionCode);
    void onWriteClicked(uint8_t functionCode);
    void onRawSendClicked();

    // Standard UI
    QSpinBox* slaveIdEdit_ = nullptr;
    QSpinBox* addressEdit_ = nullptr;
    QSpinBox* quantityEdit_ = nullptr;
    QLineEdit* writeDataEdit_ = nullptr;
    QComboBox* dataFormatBox_ = nullptr;

    // Raw UI
    QTextEdit* rawDataEdit_ = nullptr;
};

} // namespace ui::widgets
