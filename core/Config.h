/**
 * @file Config.h
 * @brief Application-wide typed configuration constants.
 *
 * Single source of truth for compile-time configuration constants, hosted in
 * the `config::` namespace.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstddef>

namespace config {

struct App {
    static constexpr char kLocaleEn[] = "en_US";
    static constexpr char kLocaleZhCn[] = "zh_CN";
    static constexpr char kLocaleZhTw[] = "zh_TW";
    static constexpr char kUpdateCheckStartup[] = "startup";
    static constexpr char kUpdateCheckWeekly[] = "weekly";
    static constexpr char kUpdateCheckMonthly[] = "monthly";
    static constexpr char kUpdateCheckNever[] = "never";
};

struct Settings {
    static constexpr int kSyncDebounceMs = 250;
};

struct Network {
    static constexpr char kDefaultDeviceAddress[] = "192.168.10.1";
    static constexpr int kDefaultModbusTcpPort = 502;
    static constexpr int kDefaultGenericTcpPort = 8080;
    static constexpr int kMinTcpPort = 1;
    static constexpr int kMaxTcpPort = 65535;
};

struct Serial {
    static constexpr char kDefaultBaudRateText[] = "9600";
    static constexpr char kDefaultDataBitsText[] = "8";
    static constexpr char kDefaultParityText[] = "None";
    static constexpr char kDefaultStopBitsText[] = "1";
    static constexpr int kDefaultBaudRate = 9600;
    static constexpr int kDefaultDataBits = 8;
    static constexpr int kDefaultStopBits = 1;
    static constexpr int kDefaultParityValue = 0;
};

struct Modbus {
    /// Maximum ADU (Application Data Unit) size in bytes.
    /// Per Modbus spec, the maximum PDU is 253 bytes; with 7 bytes MBAP overhead
    /// for TCP (Transaction ID 2 + Protocol ID 2 + Length 2 + Unit ID 1) the max
    /// TCP ADU = 7 + 253 = 260. For RTU, 1 slave + 253 PDU + 2 CRC = 256.
    /// 260 safely covers either transport.
    /// NOTE: For the MBAP Length field limit, use
    /// config::Modbus::kMaxTcpMbapLength (= 254 = Unit ID 1 + PDU 253).
    static constexpr int kMaxAduSize = 260;

    static constexpr int kDefaultSlaveId = 1;
    static constexpr int kMinSlaveId = 0;
    static constexpr int kMaxSlaveId = 247;
    static constexpr int kMinAddress = 0;
    static constexpr int kMaxAddress = 65535;
    static constexpr int kMinQuantity = 1;
    static constexpr int kMaxReadQuantity = 125;
    static constexpr int kMaxReadBitsQuantity = 2000;
    static constexpr int kMaxWriteRegistersQuantity = 123;
    static constexpr int kMaxWriteCoilsQuantity = 1968;
    static constexpr int kDefaultTimeoutMs = 1000;
    static constexpr int kDefaultRetryCount = 3;
    static constexpr int kDefaultRetryIntervalMs = 100;
    static constexpr int kDefaultMaxRetryIntervalMs = 5000;
    static constexpr double kDefaultRetryBackoffFactor = 2.0;
    static constexpr int kDefaultRetryJitterPercent = 15;
    static constexpr bool kDefaultRetryEnabled = false;
    static constexpr bool kDefaultAutoReconnect = true;
    static constexpr int kDefaultReconnectBaseMs = 250;
    static constexpr int kDefaultReconnectMaxMs = 5000;
    static constexpr int kDefaultStandardStartAddress = 0;
    static constexpr int kDefaultStandardQuantity = 10;
    static constexpr int kDefaultStandardFormatIndex = 0;
    static constexpr int kDefaultControlFunctionIndex = 2;
    static constexpr int kDefaultControlAddress = 0;
    static constexpr int kDefaultControlQuantity = 10;
    static constexpr int kDefaultControlIntervalMs = 1000;
    static constexpr int kMinTimeoutMs = 100;
    static constexpr int kMaxTimeoutMs = 60000;
    static constexpr int kTimeoutStepMs = 100;
    static constexpr int kMinRetryCount = 0;
    static constexpr int kMaxRetryCount = 10;
    static constexpr int kMinRetryIntervalMs = 10;
    static constexpr int kMaxRetryIntervalMs = 10000;
    static constexpr int kRetryIntervalStepMs = 10;
    static constexpr int kMaxDroppedInvalidBytes = 256;
    static constexpr int kMaxTcpBufferedBytes = 1024;
    static constexpr int kMaxTcpMbapLength = 254;
    static constexpr int kMaxPduDataLength = 252;
};

struct Logging {
    static constexpr int kAsyncQueueSize = 8192;
    static constexpr int kAsyncWorkerThreads = 1;
    static constexpr std::size_t kMaxFileSizeBytes = 10 * 1024 * 1024;
    static constexpr std::size_t kMaxRotatedFiles = 20;
};

struct GenericIo {
    static constexpr int kDefaultInputIntervalMs = 1000;
    static constexpr int kMinInputIntervalMs = 10;
    static constexpr int kMaxInputIntervalMs = 3600000;
    static constexpr int kFileSendChunkSizeBytes = 4096;
};

struct Polling {
    static constexpr int kDefaultIntervalMs = 1000;
    static constexpr int kMinIntervalMs = 100;
    static constexpr int kMaxIntervalMs = 60000;
    static constexpr int kIntervalStepMs = 100;
};

struct Ui {
    static constexpr int kMainWindowMinWidth = 720;
    static constexpr int kMainWindowMinHeight = 480;
    static constexpr int kMainWindowDefaultWidth = 1280;
    static constexpr int kMainWindowDefaultHeight = 800;
    static constexpr int kAboutDialogMinWidth = 420;
    static constexpr int kFrameAnalyzerDefaultHistoryWidth = 180;
    static constexpr int kFrameAnalyzerMinHistoryWidth = 120;
    static constexpr int kFrameAnalyzerDefaultInputHeight = 140;
    static constexpr int kFrameAnalyzerDefaultResultsHeight = 520;
    static constexpr int kFrameAnalyzerAdaptiveHistoryCollapseWidth = 1040;
    static constexpr int kFrameAnalyzerMaxHistoryItems = 20;
    static constexpr int kFrameAnalyzerCsvExportChunkRows = 128;
    static constexpr int kTrafficLogExportChunkRows = 256;
    static constexpr int kNavigationExpandedWidth = 138;
    static constexpr int kNavigationCollapsedWidth = 52;
    static constexpr int kNavigationMinIconWidth = 24;
    static constexpr int kNavigationLeftInset = 8;
    static constexpr int kNavigationTextGap = 6;
    static constexpr int kNavigationRightInset = 9;
    static constexpr int kTrafficMonitorMaxBlockCount = 1000;
    static constexpr int kByteMonitorMaxBlockCount = 50000;
};

struct Io {
    /// Maximum chunk size for file transfer via sendFile().
    /// Clamped to prevent memory exhaustion from unreasonable user input.
    static constexpr int kMaxChunkSizeBytes = 65536;
};

} // namespace config
