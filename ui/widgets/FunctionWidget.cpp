#include "FunctionWidget.h"
#include "CollapsibleSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QEvent>
#include <QSettings>
#include <QApplication>
#include <QSignalBlocker>

namespace ui::widgets {

FunctionWidget::FunctionWidget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

FunctionWidget::~FunctionWidget() = default;

void FunctionWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    standardSection_ = new CollapsibleSection(this);
    setupStandardUi(standardSection_->contentWidget());
    mainLayout->addWidget(standardSection_);

    rawSection_ = new CollapsibleSection(this);
    setupRawUi(rawSection_->contentWidget());
    mainLayout->addWidget(rawSection_);

    retranslateUi();
}

void FunctionWidget::setupStandardUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);
    
    // Row 1: Parameters
    auto paramLayout = new QHBoxLayout();
    
    slaveIdLabel_ = new QLabel(parent);
    paramLayout->addWidget(slaveIdLabel_);
    slaveIdEdit_ = new QSpinBox(parent);
    slaveIdEdit_->setRange(1, 247);
    slaveIdEdit_->setValue(1);
    paramLayout->addWidget(slaveIdEdit_);

    addressLabel_ = new QLabel(parent);
    paramLayout->addWidget(addressLabel_);
    addressEdit_ = new QSpinBox(parent);
    addressEdit_->setRange(0, 65535);
    addressEdit_->setValue(0);
    paramLayout->addWidget(addressEdit_);

    quantityLabel_ = new QLabel(parent);
    paramLayout->addWidget(quantityLabel_);
    quantityEdit_ = new QSpinBox(parent);
    quantityEdit_->setRange(1, 125); // Typical Modbus Limit
    quantityEdit_->setValue(10);
    paramLayout->addWidget(quantityEdit_);
    
    paramLayout->addStretch();
    layout->addLayout(paramLayout);

    // Row 2: Write Data Input
    auto writeLayout = new QHBoxLayout();
    writeDataLabel_ = new QLabel(parent);
    writeLayout->addWidget(writeDataLabel_);
    writeDataEdit_ = new QLineEdit(parent);
    writeLayout->addWidget(writeDataEdit_);
    
    formatLabel_ = new QLabel(parent);
    writeLayout->addWidget(formatLabel_);
    dataFormatBox_ = new QComboBox(parent);
    dataFormatBox_->addItem("", "Hex");
    dataFormatBox_->addItem("", "Decimal");
    writeLayout->addWidget(dataFormatBox_);
    
    layout->addLayout(writeLayout);

    // Row 3: Function Buttons
    auto btnLayout = new QGridLayout();
    
    // Read Buttons
    readBtn01_ = new QPushButton(parent);
    readBtn02_ = new QPushButton(parent);
    readBtn03_ = new QPushButton(parent);
    readBtn04_ = new QPushButton(parent);
    
    connect(readBtn01_, &QPushButton::clicked, [this](){ onReadClicked(0x01); });
    connect(readBtn02_, &QPushButton::clicked, [this](){ onReadClicked(0x02); });
    connect(readBtn03_, &QPushButton::clicked, [this](){ onReadClicked(0x03); });
    connect(readBtn04_, &QPushButton::clicked, [this](){ onReadClicked(0x04); });

    // Add buttons to layout
    btnLayout->addWidget(readBtn01_, 0, 0);
    btnLayout->addWidget(readBtn02_, 0, 1);
    btnLayout->addWidget(readBtn03_, 0, 2);
    btnLayout->addWidget(readBtn04_, 0, 3);

    // Write Buttons
    writeBtn05_ = new QPushButton(parent);
    writeBtn06_ = new QPushButton(parent);
    writeBtn0F_ = new QPushButton(parent);
    writeBtn10_ = new QPushButton(parent);

    connect(writeBtn05_, &QPushButton::clicked, [this](){ onWriteClicked(0x05); });
    connect(writeBtn06_, &QPushButton::clicked, [this](){ onWriteClicked(0x06); });
    connect(writeBtn0F_, &QPushButton::clicked, [this](){ onWriteClicked(0x0F); });
    connect(writeBtn10_, &QPushButton::clicked, [this](){ onWriteClicked(0x10); });

    btnLayout->addWidget(writeBtn05_, 1, 0);
    btnLayout->addWidget(writeBtn06_, 1, 1);
    btnLayout->addWidget(writeBtn0F_, 1, 2);
    btnLayout->addWidget(writeBtn10_, 1, 3);

    layout->addLayout(btnLayout);
    layout->addStretch();

    connect(slaveIdEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(addressEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(quantityEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &FunctionWidget::saveSettings);
    connect(dataFormatBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, &FunctionWidget::saveSettings);
}

void FunctionWidget::setupRawUi(QWidget* parent) {
    auto layout = new QVBoxLayout(parent);

    rawDataLabel_ = new QLabel(parent);
    layout->addWidget(rawDataLabel_);
    
    rawDataEdit_ = new QTextEdit(parent);
    rawDataEdit_->setMaximumHeight(96);
    layout->addWidget(rawDataEdit_);

    auto btnLayout = new QHBoxLayout();
    rawSendBtn_ = new QPushButton(parent);
    connect(rawSendBtn_, &QPushButton::clicked, this, &FunctionWidget::onRawSendClicked);
    
    btnLayout->addStretch();
    btnLayout->addWidget(rawSendBtn_);
    
    layout->addLayout(btnLayout);
}

int FunctionWidget::getSlaveId() const {
    return slaveIdEdit_ ? slaveIdEdit_->value() : 1;
}

int FunctionWidget::getStartAddress() const {
    return addressEdit_ ? addressEdit_->value() : 0;
}

int FunctionWidget::getQuantity() const {
    return quantityEdit_ ? quantityEdit_->value() : 1;
}

int FunctionWidget::getFormatIndex() const {
    return dataFormatBox_ ? dataFormatBox_->currentIndex() : 0;
}

void FunctionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (standardSection_) {
        standardSection_->setSettingsKey(settingsGroup_ + "/ui/standardCollapsed");
    }
    if (rawSection_) {
        rawSection_->setSettingsKey(settingsGroup_ + "/ui/rawCollapsed");
    }
    loadSettings();
}

void FunctionWidget::loadSettings() {
    if (settingsGroup_.isEmpty()) return;

    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QSignalBlocker b1(slaveIdEdit_);
    QSignalBlocker b2(addressEdit_);
    QSignalBlocker b3(quantityEdit_);
    QSignalBlocker b4(dataFormatBox_);

    const QString slaveKey = settingsGroup_ + "/slaveId";
    const QString addrKey = settingsGroup_ + "/startAddr";
    const QString qtyKey = settingsGroup_ + "/quantity";
    const QString formatKey = settingsGroup_ + "/formatIndex";

    const int slaveId = settings.value(slaveKey, slaveIdEdit_->value()).toInt();
    const int startAddr = settings.value(addrKey, addressEdit_->value()).toInt();
    const int quantity = settings.value(qtyKey, quantityEdit_->value()).toInt();
    const int formatIndex = settings.value(formatKey, dataFormatBox_->currentIndex()).toInt();

    slaveIdEdit_->setValue(slaveId);
    addressEdit_->setValue(startAddr);
    quantityEdit_->setValue(quantity);
    if (formatIndex >= 0 && formatIndex < dataFormatBox_->count()) {
        dataFormatBox_->setCurrentIndex(formatIndex);
    }

    if (!settings.contains(slaveKey)) settings.setValue(slaveKey, slaveId);
    if (!settings.contains(addrKey)) settings.setValue(addrKey, startAddr);
    if (!settings.contains(qtyKey)) settings.setValue(qtyKey, quantity);
    if (!settings.contains(formatKey)) settings.setValue(formatKey, dataFormatBox_->currentIndex());
}

void FunctionWidget::saveSettings() {
    if (settingsGroup_.isEmpty()) return;
    QSettings settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings.setValue(settingsGroup_ + "/slaveId", slaveIdEdit_->value());
    settings.setValue(settingsGroup_ + "/startAddr", addressEdit_->value());
    settings.setValue(settingsGroup_ + "/quantity", quantityEdit_->value());
    settings.setValue(settingsGroup_ + "/formatIndex", dataFormatBox_->currentIndex());
}

void FunctionWidget::onReadClicked(uint8_t functionCode) {
    emit readRequested(functionCode, addressEdit_->value(), quantityEdit_->value(), slaveIdEdit_->value());
}

void FunctionWidget::onWriteClicked(uint8_t functionCode) {
    emit writeRequested(functionCode, addressEdit_->value(), writeDataEdit_->text(), dataFormatBox_->currentData().toString(), slaveIdEdit_->value());
}

void FunctionWidget::onRawSendClicked() {
    QString text = rawDataEdit_->toPlainText();
    // Simple Hex parsing: remove spaces, convert to byte array
    text = text.remove(' ');
    text = text.remove('\n');
    text = text.remove('\r');
    
    QByteArray data = QByteArray::fromHex(text.toLatin1());
    if (!data.isEmpty()) {
        emit rawSendRequested(data);
    }
}

void FunctionWidget::retranslateUi() {
    if (standardSection_) {
        standardSection_->setTitle(tr("Standard"));
    }
    if (rawSection_) {
        rawSection_->setTitle(tr("Raw"));
    }
    if (slaveIdLabel_) {
        slaveIdLabel_->setText(tr("Slave ID:"));
    }
    if (addressLabel_) {
        addressLabel_->setText(tr("Start Addr:"));
    }
    if (quantityLabel_) {
        quantityLabel_->setText(tr("Quantity:"));
    }
    if (writeDataLabel_) {
        writeDataLabel_->setText(tr("Write Data:"));
    }
    if (writeDataEdit_) {
        writeDataEdit_->setPlaceholderText(tr("Space separated hex (e.g., 01 02) or values"));
    }
    if (formatLabel_) {
        formatLabel_->setText(tr("Format:"));
    }
    if (dataFormatBox_) {
        dataFormatBox_->setItemText(0, tr("Hex"));
        dataFormatBox_->setItemText(1, tr("Decimal"));
    }
    if (readBtn01_) {
        readBtn01_->setText(tr("Read Coils (0x01)"));
    }
    if (readBtn02_) {
        readBtn02_->setText(tr("Read Discrete (0x02)"));
    }
    if (readBtn03_) {
        readBtn03_->setText(tr("Read Holding (0x03)"));
    }
    if (readBtn04_) {
        readBtn04_->setText(tr("Read Input (0x04)"));
    }
    if (writeBtn05_) {
        writeBtn05_->setText(tr("Write Coil (0x05)"));
    }
    if (writeBtn06_) {
        writeBtn06_->setText(tr("Write Reg (0x06)"));
    }
    if (writeBtn0F_) {
        writeBtn0F_->setText(tr("Write Multi Coils (0x0F)"));
    }
    if (writeBtn10_) {
        writeBtn10_->setText(tr("Write Multi Regs (0x10)"));
    }
    if (rawDataLabel_) {
        rawDataLabel_->setText(tr("Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):"));
    }
    if (rawSendBtn_) {
        rawSendBtn_->setText(tr("Send Raw"));
    }
}

void FunctionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
