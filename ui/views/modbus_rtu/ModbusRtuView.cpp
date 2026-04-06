#include "ModbusRtuView.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include "modbus/factory/ModbusFactory.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <spdlog/spdlog.h>
#include <QMetaObject>
#include <QPointer>
#include <limits>

namespace ui::views::modbus_rtu {

ModbusRtuView::ModbusRtuView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    setupUi();
}

ModbusRtuView::~ModbusRtuView() {
    releaseStack();
}

void ModbusRtuView::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    timeoutMs_ = timeoutMs;
    retries_ = retries;
    retryIntervalMs_ = retryIntervalMs;
    if (client_) {
        currentConfig_.timeoutMs = timeoutMs_;
        currentConfig_.retries = retries_;
        currentConfig_.retryIntervalMs = retryIntervalMs_;
        client_->setConfig(currentConfig_);
    }
}

void ModbusRtuView::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);
    
    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup("modbus/rtu/serial");
    mainLayout_->addWidget(connectionWidget_);
    
    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup("modbus/rtu/standard");
    mainLayout_->addWidget(functionWidget_);
    
    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey("modbus/rtu/ui/dataMonitorCollapsed");
    auto dataLayout = new QHBoxLayout(dataGroup_->contentWidget());
    dataLayout->setContentsMargins(4, 0, 4, 4);
    dataLayout->setSpacing(4);

    receiveGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto receiveLayout = new QVBoxLayout(receiveGroup_);
    auto receiveToolbar = new QHBoxLayout();
    receiveHexCheck_ = new QCheckBox(receiveGroup_);
    receiveHexCheck_->setChecked(true);
    receiveHexCheck_->setVisible(false);
    copyReceiveButton_ = new QPushButton(receiveGroup_);
    clearReceiveButton_ = new QPushButton(receiveGroup_);
    receiveToolbar->addWidget(receiveHexCheck_);
    receiveToolbar->addStretch();
    receiveToolbar->addWidget(copyReceiveButton_);
    receiveToolbar->addWidget(clearReceiveButton_);
    receiveTextEdit_ = new QTextEdit(receiveGroup_);
    receiveTextEdit_->setReadOnly(true);
    receiveTextEdit_->document()->setMaximumBlockCount(app::constants::Constants::Ui::kTrafficMonitorMaxBlockCount);
    receiveLayout->addLayout(receiveToolbar);
    receiveLayout->addWidget(receiveTextEdit_);

    sendGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto sendLayout = new QVBoxLayout(sendGroup_);
    auto sendToolbar = new QHBoxLayout();
    sendHexCheck_ = new QCheckBox(sendGroup_);
    sendHexCheck_->setChecked(true);
    sendHexCheck_->setVisible(false);
    copySendButton_ = new QPushButton(sendGroup_);
    clearSendButton_ = new QPushButton(sendGroup_);
    sendToolbar->addWidget(sendHexCheck_);
    sendToolbar->addStretch();
    sendToolbar->addWidget(copySendButton_);
    sendToolbar->addWidget(clearSendButton_);
    sendTextEdit_ = new QTextEdit(sendGroup_);
    sendTextEdit_->setReadOnly(true);
    sendTextEdit_->document()->setMaximumBlockCount(app::constants::Constants::Ui::kTrafficMonitorMaxBlockCount);
    sendLayout->addLayout(sendToolbar);
    sendLayout->addWidget(sendTextEdit_);

    dataLayout->addWidget(receiveGroup_, 1);
    dataLayout->addWidget(sendGroup_, 1);
    trafficMonitor_ = new widgets::TrafficMonitorWidget(settingsService_, this);
    trafficMonitor_->setMinimumHeight(140);
    trafficMonitor_->setSettingsGroup("modbus/rtu/traffic");

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);
    
    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup("modbus/rtu/control");
    mainLayout_->addWidget(controlWidget_);

    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked, 
        [this](const io::SerialConfig& config) {
            spdlog::info("ModbusRtuView: Connect requested to {}", config.portName.toStdString());
            trafficMonitor_->appendInfo(tr("Opening %1...").arg(config.portName));

            releaseStack();

            modbus::base::ModbusConfig modbusConfig;
            modbusConfig.mode = modbus::base::ModbusMode::RTU;
            modbusConfig.portName = config.portName;
            modbusConfig.baudRate = config.baudRate;
            modbusConfig.dataBits = config.dataBits;
            modbusConfig.stopBits = static_cast<int>(config.stopBits);
            modbusConfig.parity = static_cast<int>(config.parity);
            modbusConfig.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
            modbusConfig.timeoutMs = timeoutMs_;
            modbusConfig.retries = retries_;
            modbusConfig.retryIntervalMs = retryIntervalMs_;

            modbus::factory::ModbusFactory factory;
            auto stack = factory.createStack(modbusConfig);
            if (!stack.worker || !stack.thread) {
                trafficMonitor_->appendInfo(tr("Failed to create Modbus stack"));
                return;
            }

            currentConfig_ = modbusConfig;
            channel_ = std::move(stack.channel);
            client_ = std::move(stack.client);
            worker_ = std::move(stack.worker);
            workerThread_ = std::move(stack.thread);

            QPointer<ModbusRtuView> self(this);
            channel_->setMonitor([self](bool isTx, const QByteArray& data) {
                if (!self) return;
                QMetaObject::invokeMethod(self, [self, isTx, data]() {
                    if (!self) return;
                    if (isTx) {
                        self->trafficMonitor_->appendTx(data);
                        self->appendSendData(data);
                    } else {
                        self->trafficMonitor_->appendRx(data);
                        self->appendReceiveData(data);
                    }
                }, Qt::QueuedConnection);
            });

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::connectFinished, this,
                [this, self](bool ok, const QString& error) {
                    if (!self) return;
                    if (ok) {
                        connectionWidget_->setConnected(true);
                        trafficMonitor_->appendInfo(tr("Connected"));
                    } else {
                        connectionWidget_->setConnected(false);
                        trafficMonitor_->appendInfo(tr("Connection failed: %1").arg(error));
                    }
                }, Qt::QueuedConnection);

            connect(worker_.get(), &modbus::dispatch::ModbusWorker::requestFinished, this,
                [this, self](int requestId, const modbus::session::ModbusResponse& response) {
                    if (!self) return;
                    auto itStart = requestStart_.find(requestId);
                    auto itKind = requestKinds_.find(requestId);
                    if (itStart == requestStart_.end() || itKind == requestKinds_.end()) {
                        return;
                    }

                    // 分流设计：仅在成功时通过底层测量的精确 RTT 更新统计
                    if (response.isSuccess) {
                        // 更新成功统计 (RX + 1, 更新 RTT)
                        controlWidget_->updateStats(false, response.rttMs);

                        if (itKind->second == RequestKind::Read) {
                            trafficMonitor_->appendInfo(tr("Success: Response received"));
                        } else if (itKind->second == RequestKind::Write) {
                            trafficMonitor_->appendInfo(tr("Success: Write confirmed"));
                        }
                    } else {
                        // 失败路径：仅更新 Error 计数，跳过 RTT 统计防止均值偏移
                        controlWidget_->updateStats(false, -1, true);

                        if (itKind->second == RequestKind::Poll) {
                            trafficMonitor_->appendInfo(tr("Poll Error: %1").arg(response.error));
                        } else {
                            trafficMonitor_->appendInfo(tr("Error: %1").arg(response.error));
                        }
                    }

                    requestStart_.erase(itStart);
                    requestKinds_.erase(itKind);
                }, Qt::QueuedConnection);

            worker_->start();
            worker_->requestConnect();
    });

    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
        [this]() {
            spdlog::info("ModbusRtuView: Disconnect requested");
            trafficMonitor_->appendInfo(tr("Disconnected"));
            releaseStack();
            connectionWidget_->setConnected(false);
    });

    auto ensureConnected = [this]() {
        if (worker_ && client_ && client_->isConnected()) {
            return true;
        }
        ui::common::ConnectionAlert::showNotConnected(this);
        return false;
    };

    connect(functionWidget_, &widgets::FunctionWidget::readRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty, int slaveId) {
            if (!ensureConnected()) return;
            
            using namespace modbus::base;
            QByteArray data;
            data.resize(4);
            data[0] = (addr >> 8) & 0xFF;
            data[1] = addr & 0xFF;
            data[2] = (qty >> 8) & 0xFF;
            data[3] = qty & 0xFF;
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(tr("Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4")
                .arg(fc).arg(addr).arg(qty).arg(slaveId));

            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Read;
            
            controlWidget_->updateStats(true, -1); // 提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
    });

    connect(functionWidget_, &widgets::FunctionWidget::writeRequested,
        [this, ensureConnected](uint8_t fc, int addr, const QString& dataStr, const QString& fmt, int slaveId) {
            if (!ensureConnected()) return;

            using namespace modbus::base;
            
            QByteArray data;
            
            data.append((char)((addr >> 8) & 0xFF));
            data.append((char)(addr & 0xFF));

            QString trimmed = dataStr.trimmed();
            auto parseHexBytes = [](const QString& input) {
                QString cleaned = input;
                cleaned.remove(QRegularExpression("[^0-9A-Fa-f]"));
                if (cleaned.isEmpty()) return QByteArray();
                if (cleaned.size() % 2 == 1) cleaned.prepend("0");
                return QByteArray::fromHex(cleaned.toLatin1());
            };
            auto parseDecimalList = [](const QString& input, bool& okOut) {
                okOut = true;
                QList<quint16> values;
                QStringList parts = input.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
                if (parts.isEmpty()) {
                    okOut = false;
                    return values;
                }
                for (const auto& part : parts) {
                    bool ok = false;
                    uint value = part.toUInt(&ok, 10);
                    if (!ok || value > 65535) {
                        okOut = false;
                        return QList<quint16>();
                    }
                    values.append(static_cast<quint16>(value));
                }
                return values;
            };

            if (fc == 0x05) {
                bool coilOn = false;
                if (fmt == "Decimal") {
                    bool ok = false;
                    int value = trimmed.toInt(&ok);
                    if (!ok || (value != 0 && value != 1)) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal value for 0x05"));
                        return;
                    }
                    coilOn = (value != 0);
                } else {
                    QByteArray bytes = parseHexBytes(trimmed);
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                        return;
                    }
                    if (bytes.size() == 1) {
                        uint8_t raw = static_cast<uint8_t>(bytes[0]);
                        if (raw != 0x00 && raw != 0x01) {
                            trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                            return;
                        }
                        coilOn = raw != 0x00;
                    } else {
                        uint16_t raw = (static_cast<uint8_t>(bytes[0]) << 8) | static_cast<uint8_t>(bytes[1]);
                        if (raw != 0x0000 && raw != 0xFF00) {
                            trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x05"));
                            return;
                        }
                        coilOn = raw == 0xFF00;
                    }
                }
                data.append(static_cast<char>(coilOn ? 0xFF : 0x00));
                data.append(static_cast<char>(0x00));
            } else if (fc == 0x06) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo(tr("Error: Empty value for 0x06"));
                    return;
                }
                if (fmt == "Decimal") {
                    bool ok = false;
                    uint value = trimmed.toUInt(&ok, 10);
                    if (!ok || value > 65535) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal value for 0x06"));
                        return;
                    }
                    data.append(static_cast<char>((value >> 8) & 0xFF));
                    data.append(static_cast<char>(value & 0xFF));
                } else {
                    QByteArray bytes = parseHexBytes(trimmed);
                    if (bytes.isEmpty()) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x06"));
                        return;
                    }
                    if (bytes.size() == 1) {
                        data.append(static_cast<char>(0x00));
                        data.append(bytes[0]);
                    } else if (bytes.size() == 2) {
                        data.append(bytes[0]);
                        data.append(bytes[1]);
                    } else {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x06"));
                        return;
                    }
                }
            } else if (fc == 0x0F) {
                if (fmt != "Hex") {
                    trafficMonitor_->appendInfo(tr("Error: 0x0F requires Hex data"));
                    return;
                }
                QByteArray bytes = parseHexBytes(trimmed);
                if (bytes.isEmpty()) {
                    trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x0F"));
                    return;
                }
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo(tr("Error: Invalid quantity for 0x0F"));
                    return;
                }
                int byteCount = (quantity + 7) / 8;
                if (bytes.size() != byteCount) {
                    trafficMonitor_->appendInfo(tr("Error: Coil data length mismatch for 0x0F"));
                    return;
                }
                data.append(static_cast<char>((quantity >> 8) & 0xFF));
                data.append(static_cast<char>(quantity & 0xFF));
                data.append(static_cast<char>(byteCount));
                data.append(bytes);
            } else if (fc == 0x10) {
                if (trimmed.isEmpty()) {
                    trafficMonitor_->appendInfo(tr("Error: Empty value for 0x10"));
                    return;
                }
                int quantity = functionWidget_->getQuantity();
                if (quantity <= 0) {
                    trafficMonitor_->appendInfo(tr("Error: Invalid quantity for 0x10"));
                    return;
                }
                QByteArray payload;
                int registerCount = 0;
                if (fmt == "Decimal") {
                    bool okList = false;
                    auto values = parseDecimalList(trimmed, okList);
                    if (!okList) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid decimal list for 0x10"));
                        return;
                    }
                    registerCount = values.size();
                    for (quint16 value : values) {
                        payload.append(static_cast<char>((value >> 8) & 0xFF));
                        payload.append(static_cast<char>(value & 0xFF));
                    }
                } else {
                    payload = parseHexBytes(trimmed);
                    if (payload.isEmpty() || (payload.size() % 2 != 0)) {
                        trafficMonitor_->appendInfo(tr("Error: Invalid hex value for 0x10"));
                        return;
                    }
                    registerCount = payload.size() / 2;
                }
                if (registerCount != quantity) {
                    trafficMonitor_->appendInfo(tr("Error: Quantity does not match data length for 0x10"));
                    return;
                }
                data.append(static_cast<char>((quantity >> 8) & 0xFF));
                data.append(static_cast<char>(quantity & 0xFF));
                data.append(static_cast<char>(registerCount * 2));
                data.append(payload);
            } else {
                trafficMonitor_->appendInfo(tr("Error: Unsupported write function code"));
                return;
            }
            
            Pdu request(static_cast<FunctionCode>(fc), data);

            trafficMonitor_->appendInfo(tr("Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4")
                .arg(fc).arg(addr).arg(dataStr).arg(slaveId));

            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Write;

            controlWidget_->updateStats(true, -1); // 提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
    });
    
    connect(functionWidget_, &widgets::FunctionWidget::rawSendRequested,
        [this, ensureConnected](const QByteArray& data) {
            if (!ensureConnected()) return;
            if (data.isEmpty()) return;
            
            trafficMonitor_->appendInfo(tr("Sending Raw Data: %1").arg(QString(data.toHex(' ').toUpper())));
            
            worker_->sendRaw(data);
            
            controlWidget_->updateStats(true, -1);
    });

    connect(controlWidget_, &widgets::ControlWidget::pollRequested,
        [this, ensureConnected](uint8_t fc, int addr, int qty) {
            if (!ensureConnected()) return;
            
            int slaveId = functionWidget_->getSlaveId();
            
            using namespace modbus::base;
            QByteArray data;
            data.resize(4);
            data[0] = (addr >> 8) & 0xFF;
            data[1] = addr & 0xFF;
            data[2] = (qty >> 8) & 0xFF;
            data[3] = qty & 0xFF;
            
            Pdu request(static_cast<FunctionCode>(fc), data);
            
            int requestId = nextRequestId();
            requestStart_[requestId] = std::chrono::steady_clock::now();
            requestKinds_[requestId] = RequestKind::Poll;

            controlWidget_->updateStats(true, -1); // 轮询提交时增加 TX 计数
            worker_->submit(request, slaveId, requestId);
    });

    connect(clearReceiveButton_, &QPushButton::clicked, [this]() {
        lastReceiveFrame_.clear();
        if (receiveTextEdit_) receiveTextEdit_->clear();
    });
    connect(clearSendButton_, &QPushButton::clicked, [this]() {
        lastSendFrame_.clear();
        if (sendTextEdit_) sendTextEdit_->clear();
    });
    connect(copyReceiveButton_, &QPushButton::clicked, [this]() {
        if (!receiveTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(receiveTextEdit_->toPlainText());
    });
    connect(copySendButton_, &QPushButton::clicked, [this]() {
        if (!sendTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(sendTextEdit_->toPlainText());
    });
    connect(receiveHexCheck_, &QCheckBox::toggled, this, [this]() {
        refreshReceiveDisplay();
    });
    connect(sendHexCheck_, &QCheckBox::toggled, this, [this]() {
        refreshSendDisplay();
    });
    
    mainLayout_->addStretch();

    retranslateUi();
}

QString ModbusRtuView::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void ModbusRtuView::releaseStack() {
    if (worker_) {
        worker_->stop();
    }
    worker_.reset();
    client_.reset();
    channel_.reset();
    workerThread_.reset();
    requestStart_.clear();
    requestKinds_.clear();
}

int ModbusRtuView::nextRequestId() {
    if (requestId_ == std::numeric_limits<int>::max()) {
        requestId_ = 0;
    }
    return ++requestId_;
}

void ModbusRtuView::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void ModbusRtuView::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void ModbusRtuView::refreshReceiveDisplay() {
    if (!receiveTextEdit_) return;
    if (lastReceiveFrame_.isEmpty()) {
        receiveTextEdit_->clear();
        return;
    }
    bool hex = receiveHexCheck_ ? receiveHexCheck_->isChecked() : true;
    receiveTextEdit_->setPlainText(tr("[%1] %2").arg(tr("RX")).arg(formatData(lastReceiveFrame_, hex)));
}

void ModbusRtuView::refreshSendDisplay() {
    if (!sendTextEdit_) return;
    if (lastSendFrame_.isEmpty()) {
        sendTextEdit_->clear();
        return;
    }
    bool hex = sendHexCheck_ ? sendHexCheck_->isChecked() : true;
    sendTextEdit_->setPlainText(tr("[%1] %2").arg(tr("TX")).arg(formatData(lastSendFrame_, hex)));
}

void ModbusRtuView::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (receiveHexCheck_) receiveHexCheck_->setText(tr("HEX"));
    if (sendHexCheck_) sendHexCheck_->setText(tr("HEX"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
    refreshReceiveDisplay();
    refreshSendDisplay();
}

void ModbusRtuView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::modbus_rtu
