#include "application/AnalyzerLinkCoordinator.h"

#include "AnalyzerLinkageController.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "widgets/FrameAnalyzerWidget.h"

namespace ui::application {

AnalyzerLinkCoordinator::AnalyzerLinkCoordinator(QObject* parent)
    : QObject(parent),
      controller_(std::make_unique<ui::AnalyzerLinkageController>(this)) {}

AnalyzerLinkCoordinator::~AnalyzerLinkCoordinator() = default;

void AnalyzerLinkCoordinator::bind(views::modbus_tcp::ModbusTcpView* tcpView,
                                   views::modbus_rtu::ModbusRtuView* rtuView,
                                   widgets::FrameAnalyzerWidget* frameAnalyzer) {
    controller_->bind(tcpView, rtuView, frameAnalyzer);

    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpView::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleTcpLinkageToggled);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuView::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleRtuLinkageToggled);
    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpView::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuView::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpView::linkageSourceDisconnected,
                     this, &AnalyzerLinkCoordinator::handleTcpSourceDisconnected);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuView::linkageSourceDisconnected,
                     this, &AnalyzerLinkCoordinator::handleRtuSourceDisconnected);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkagePauseToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkagePauseToggled);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkageStopRequested,
                     this, &AnalyzerLinkCoordinator::handleLinkageStopRequested);
}

void AnalyzerLinkCoordinator::handleTcpLinkageToggled(bool active) {
    controller_->requestLinkToggle(ui::AnalyzerLinkageController::LinkSource::Tcp, active);
}

void AnalyzerLinkCoordinator::handleRtuLinkageToggled(bool active) {
    controller_->requestLinkToggle(ui::AnalyzerLinkageController::LinkSource::Rtu, active);
}

void AnalyzerLinkCoordinator::handleLinkageStopRequested() {
    controller_->requestStop();
}

void AnalyzerLinkCoordinator::handleLinkagePauseToggled(bool paused) {
    controller_->requestPause(paused);
}

void AnalyzerLinkCoordinator::handleLinkageData(const modbus::base::Pdu& pdu,
                                                modbus::core::parser::ProtocolType protocol,
                                                uint16_t addr) {
    controller_->handleLiveData(pdu, protocol, addr);
}

void AnalyzerLinkCoordinator::handleTcpSourceDisconnected() {
    controller_->handleSourceDisconnected(ui::AnalyzerLinkageController::LinkSource::Tcp);
}

void AnalyzerLinkCoordinator::handleRtuSourceDisconnected() {
    controller_->handleSourceDisconnected(ui::AnalyzerLinkageController::LinkSource::Rtu);
}

} // namespace ui::application
