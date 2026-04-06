#pragma once

namespace app::constants {

class Constants final {
public:
    struct Settings {
        inline static constexpr int kSyncDebounceMs = 250;
    };

    struct Network {
        inline static constexpr char kLoopbackAddress[] = "127.0.0.1";
        inline static constexpr int kDefaultModbusTcpPort = 502;
        inline static constexpr int kDefaultGenericTcpPort = 8080;
        inline static constexpr int kMinTcpPort = 1;
        inline static constexpr int kMaxTcpPort = 65535;
    };

    struct Serial {
        inline static constexpr char kDefaultBaudRateText[] = "9600";
        inline static constexpr char kDefaultDataBitsText[] = "8";
        inline static constexpr char kDefaultParityText[] = "None";
        inline static constexpr char kDefaultStopBitsText[] = "1";
        inline static constexpr int kDefaultBaudRate = 9600;
        inline static constexpr int kDefaultDataBits = 8;
        inline static constexpr int kDefaultStopBits = 1;
        inline static constexpr int kDefaultParityValue = 0;
    };

    struct Modbus {
        inline static constexpr int kDefaultSlaveId = 1;
        inline static constexpr int kMinSlaveId = 1;
        inline static constexpr int kMaxSlaveId = 247;
        inline static constexpr int kMinAddress = 0;
        inline static constexpr int kMaxAddress = 65535;
        inline static constexpr int kMinQuantity = 1;
        inline static constexpr int kMaxReadQuantity = 125;
        inline static constexpr int kDefaultTimeoutMs = 1000;
        inline static constexpr int kDefaultRetryCount = 3;
        inline static constexpr int kDefaultRetryIntervalMs = 100;
        inline static constexpr bool kDefaultRetryEnabled = false;
        inline static constexpr int kDefaultStandardStartAddress = 0;
        inline static constexpr int kDefaultStandardQuantity = 10;
        inline static constexpr int kDefaultStandardFormatIndex = 0;
        inline static constexpr int kDefaultControlFunctionIndex = 0;
        inline static constexpr int kDefaultControlAddress = 0;
        inline static constexpr int kDefaultControlQuantity = 10;
        inline static constexpr int kDefaultControlIntervalMs = 1000;
        inline static constexpr int kMinTimeoutMs = 100;
        inline static constexpr int kMaxTimeoutMs = 60000;
        inline static constexpr int kTimeoutStepMs = 100;
        inline static constexpr int kMinRetryCount = 0;
        inline static constexpr int kMaxRetryCount = 10;
        inline static constexpr int kMinRetryIntervalMs = 10;
        inline static constexpr int kMaxRetryIntervalMs = 10000;
        inline static constexpr int kRetryIntervalStepMs = 10;
        inline static constexpr int kMaxDroppedInvalidBytes = 256;
    };

    struct GenericIo {
        inline static constexpr int kDefaultInputIntervalMs = 1000;
        inline static constexpr int kMinInputIntervalMs = 10;
        inline static constexpr int kMaxInputIntervalMs = 3600000;
    };

    struct Polling {
        inline static constexpr int kDefaultIntervalMs = 1000;
        inline static constexpr int kMinIntervalMs = 10;
        inline static constexpr int kMaxIntervalMs = 60000;
        inline static constexpr int kIntervalStepMs = 100;
    };

    struct Ui {
        inline static constexpr int kFrameAnalyzerAdaptiveHistoryCollapseWidth = 1040;
        inline static constexpr int kFrameAnalyzerMaxHistoryItems = 20;
        inline static constexpr int kTrafficMonitorMaxBlockCount = 1000;
    };
};

} // namespace app::constants
