#pragma once

#include <QWidget>
#include <cstdint>

class QSpinBox;
class QLineEdit;
class QComboBox;

namespace ui::widgets {

class FunctionWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionWidget(QWidget *parent = nullptr);
    ~FunctionWidget() override;

signals:
    // Read: Function Code, Address, Quantity
    void readRequested(uint8_t functionCode, int address, int quantity);
    
    // Write: Function Code, Address, Data (as string, parsed internally or by controller)
    // For simplicity, we pass the raw string and format.
    void writeRequested(uint8_t functionCode, int address, const QString& data, const QString& format);

private:
    void setupUi();
    void onReadClicked(uint8_t functionCode);
    void onWriteClicked(uint8_t functionCode);

    QSpinBox* slaveIdEdit_ = nullptr;
    QSpinBox* addressEdit_ = nullptr;
    QSpinBox* quantityEdit_ = nullptr;
    QLineEdit* writeDataEdit_ = nullptr;
    QComboBox* dataFormatBox_ = nullptr;
};

} // namespace ui::widgets
