// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <functional>
#include "application/modbus/ModbusExecuteSpec.h"
#include "application/modbus/IModbusPageView.h"
#include "modbus/base/ModbusAddressMapping.h"
#include "modbus/base/ModbusFunctionTraits.h"
#include "modbus/parser/ModbusFrameParser.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QStackedLayout;
class QVBoxLayout;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets::modbus {

/// Role: Master-detail workbench controller presenting standard function code operations and raw frame dispatch.
/// Thread-safety: Thread-affine (must be invoked on GUI main thread).
/// Ownership: Owned by views::modbus::ModbusPage via QObject parent-child hierarchy.
class ModbusWorkbenchWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModbusWorkbenchWidget(infra::config::ISettingsService* settingsService,
                                   QWidget* parent = nullptr);
    ~ModbusWorkbenchWidget() noexcept override;

    ModbusWorkbenchWidget(const ModbusWorkbenchWidget&) = delete;
    ModbusWorkbenchWidget& operator=(const ModbusWorkbenchWidget&) = delete;
    ModbusWorkbenchWidget(ModbusWorkbenchWidget&&) noexcept = delete;
    ModbusWorkbenchWidget& operator=(ModbusWorkbenchWidget&&) noexcept = delete;

    // View synchronization API
    void setExecutionGuards(const ::ui::application::modbus::ExecutionGuardState& guards);
    void setAddressBase(::modbus::address::AddressBase base);
    void setProtocolType(::modbus::parser::ProtocolType protocol);

    // Testing hook for Defense 1 modal confirmation
    void setConfirmWritePollHandler(std::function<bool(uint8_t fc)> handler) noexcept;

    // State getters
    [[nodiscard]] ::ui::application::modbus::ModbusExecuteSpec currentSpec() const;
    [[nodiscard]] bool isRawMode() const noexcept { return isRawMode_; }
    [[nodiscard]] bool isPolling() const noexcept { return isPolling_; }
    [[nodiscard]] int pollingIntervalMs() const noexcept;
    [[nodiscard]] uint8_t slaveId() const noexcept;
    [[nodiscard]] uint8_t functionCode() const noexcept;
    [[nodiscard]] uint16_t startAddress() const noexcept { return pduAddress_; }
    [[nodiscard]] uint16_t quantity() const noexcept;
    [[nodiscard]] int radix() const noexcept { return radix_; }
    [[nodiscard]] int dataRadix() const noexcept { return dataRadix_; }
    [[nodiscard]] ::modbus::address::AddressBase addressBase() const noexcept { return addressBase_; }
    [[nodiscard]] QComboBox* addressBaseComboBox() const noexcept { return addrBaseCombo_; }
    [[nodiscard]] QLineEdit* addressEdit() const noexcept { return addressEdit_; }
    [[nodiscard]] QLabel* addressPreviewLabel() const noexcept { return addressPreviewLabel_; }
    [[nodiscard]] QLabel* addressWarningLabel() const noexcept { return addressWarningLabel_; }
    [[nodiscard]] QLabel* writeWarningLabel() const noexcept { return writeWarningLabel_; }
    [[nodiscard]] QGroupBox* writeGroup() const noexcept { return writeGroup_; }
    [[nodiscard]] QPushButton* executeButton() const noexcept { return executeBtn_; }
    [[nodiscard]] QLabel* guardHintLabel() const noexcept { return guardHintLabel_; }

    // State setters (programmatic / testing)
    void setMode(int mode);
    void setSlaveId(uint8_t slaveId);
    void setFunctionCode(uint8_t fc);
    void setStartAddress(uint16_t addr);
    void setQuantity(uint16_t qty);
    void setRadix(int radix);
    void setDataRadix(int radix);
    void setPollingIntervalMs(int ms);
    void setFc05Value(bool on);
    void setWritePayload(const QString& text);
    void setRawText(const QString& text);
    void setPollingChecked(bool checked);

signals:
    void executeRequested(const ::ui::application::modbus::ModbusExecuteSpec& spec);
    void pollToggleRequested(bool start, const ::ui::application::modbus::ModbusExecuteSpec& spec, bool userConfirmedWritePoll);
    void pollIntervalChanged(int intervalMs);
    void logMessageRequested(const QString& message, bool isError);
    void addressBaseChanged(::modbus::address::AddressBase base);

private slots:
    void onModeChanged(int id);
    void onRadixChanged(int id);
    void onDataRadixChanged(int id);
    void onFunctionCodeChanged(int index);
    void onSlaveIdChanged(int val);
    void onAddressEdited();
    void onQuantityChanged(int val);
    void onFc05Toggled(bool on);
    void onExecuteClicked();
    void onPollCheckBoxToggled(bool checked);
    void onPollIntervalChanged(int val);
    void onAddMbapClicked();
    void onAddCrcClicked();
    void onAddLrcClicked();
    void onCleanHexClicked();

private:
    void setupUi();
    QWidget* createStandardWidget();
    QWidget* createRawWidget();
    void setupAddressingSection(QWidget* parent, QVBoxLayout* layout);
    void setupFunctionCodeSelector(QGroupBox* group, QVBoxLayout* gl);
    void setupAddressingForm(QGroupBox* group, QVBoxLayout* gl);
    void setupRadixRow(QGroupBox* group, QFormLayout* form);
    void setupAddressBaseRow(QGroupBox* group, QFormLayout* form);
    void setupSlaveIdRow(QGroupBox* group, QFormLayout* form);
    void setupAddressAndQuantityRows(QGroupBox* group, QFormLayout* form);
    void setupModeToggle(QWidget* container, QVBoxLayout* rootLayout);
    void setupWriteSection(QWidget* parent, QVBoxLayout* layout);
    void setupDataRadixRow(QGroupBox* group, QVBoxLayout* wl);
    void setupWriteStackPages(QWidget* stackWidget);
    void setupExecutionSection(QWidget* parent, QVBoxLayout* layout);
    void setupPollControls(QVBoxLayout* gl);
    void setupRawButtons(QGroupBox* editGroup, QVBoxLayout* el);
    void updateExecuteButtonAppearance();
    void updateGuardAppearance();
    void updateAddressDisplay();
    void updateRequestDataState(uint8_t fc);
    void updateDataRadixHints();
    void validateInputs();
    void validateAddressRange(bool& valid, QString& errorMsg);
    void validateWriteData(bool& valid, QString& errorMsg);
    void updateWarningAndActionState(
        bool addrValid, const QString& addrError, bool writeValid, const QString& writeError, uint8_t sid);
    bool confirmWritePoll(uint8_t fc);
    void stopPollingInternal(bool emitSignal);
    void loadSettings();
    void loadAddressingSettings();
    void loadExecutionAndPayloadSettings();
    void saveSettings();
    void retranslateUi();
    void retranslateAddressingUi();
    void retranslateWriteAndRawUi();
    void retranslateExecutionUi();
    void changeEvent(QEvent* event) override;

    infra::config::ISettingsService* settingsService_ = nullptr;
    ::ui::application::modbus::ExecutionGuardState guardState_;
    ::modbus::address::AddressBase addressBase_ = ::modbus::address::AddressBase::Offset0Based;
    ::modbus::parser::ProtocolType protocolType_ = ::modbus::parser::ProtocolType::Tcp;
    std::function<bool(uint8_t fc)> confirmWritePollHandler_;

    // State variables
    bool isRawMode_ = false;
    bool isPolling_ = false;
    bool userConfirmedWritePoll_ = false;
    uint16_t pduAddress_ = 0;
    int radix_ = 10;
    int dataRadix_ = 10;
    int registerDataRadix_ = 10;
    bool writePayloadDirty_ = false;
    bool fc05IsOn_ = true;

    // UI Widgets
    QButtonGroup* modeGroup_ = nullptr;
    QPushButton* standardModeBtn_ = nullptr;
    QPushButton* rawModeBtn_ = nullptr;
    QStackedLayout* mainStackedLayout_ = nullptr;

    // Standard mode widgets
    QGroupBox* addressingGroup_ = nullptr;
    QLabel* radixLabel_ = nullptr;
    QButtonGroup* radixGroup_ = nullptr;
    QPushButton* radixDecBtn_ = nullptr;
    QPushButton* radixHexBtn_ = nullptr;
    QLabel* addrBaseLabel_ = nullptr;
    QComboBox* addrBaseCombo_ = nullptr;
    QLabel* fcLabel_ = nullptr;
    QComboBox* fcCombo_ = nullptr;
    QLabel* slaveIdLabel_ = nullptr;
    QSpinBox* slaveIdSpin_ = nullptr;
    QLabel* startAddrLabel_ = nullptr;
    QLineEdit* addressEdit_ = nullptr;
    QLabel* addressPreviewLabel_ = nullptr;
    QLabel* quantityLabel_ = nullptr;
    QSpinBox* quantitySpin_ = nullptr;
    QLabel* addressWarningLabel_ = nullptr;

    // Write stack
    QGroupBox* writeGroup_ = nullptr;
    QLabel* dataRadixLabel_ = nullptr;
    QButtonGroup* dataRadixGroup_ = nullptr;
    QPushButton* dataRadixDecBtn_ = nullptr;
    QPushButton* dataRadixHexBtn_ = nullptr;
    QPushButton* dataRadixBinBtn_ = nullptr;
    QStackedLayout* writeStackedLayout_ = nullptr;
    QLabel* readPlaceholderLabel_ = nullptr;
    QPushButton* fc05OnBtn_ = nullptr;
    QPushButton* fc05OffBtn_ = nullptr;
    QButtonGroup* fc05Group_ = nullptr;
    QLabel* fc06ValueLabel_ = nullptr;
    QLineEdit* fc06ValueEdit_ = nullptr;
    QLineEdit* fc0fBitsEdit_ = nullptr;
    QLabel* fc0fHintLabel_ = nullptr;
    QLineEdit* fc10WordsEdit_ = nullptr;
    QLabel* fc10HintLabel_ = nullptr;
    QLabel* writeWarningLabel_ = nullptr;

    // Execution & polling
    QGroupBox* executionGroup_ = nullptr;
    QPushButton* executeBtn_ = nullptr;
    QCheckBox* pollCheckBox_ = nullptr;
    QLabel* pollIntervalLabel_ = nullptr;
    QSpinBox* pollIntervalSpin_ = nullptr;
    QLabel* writePollWarningBadge_ = nullptr;
    QLabel* guardHintLabel_ = nullptr;

    // Raw mode widgets
    QGroupBox* rawGroup_ = nullptr;
    QPlainTextEdit* rawTextEdit_ = nullptr;
    QPushButton* addMbapBtn_ = nullptr;
    QPushButton* addCrcBtn_ = nullptr;
    QPushButton* addLrcBtn_ = nullptr;
    QPushButton* cleanHexBtn_ = nullptr;
    QPushButton* rawExecuteBtn_ = nullptr;
};

} // namespace ui::widgets::modbus
