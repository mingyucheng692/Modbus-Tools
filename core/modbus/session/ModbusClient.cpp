/**
 * @file ModbusClient.cpp
 * @brief Implementation of ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusClient.h"
#include "AppConstants.h"
#include "../base/ModbusProtocolChecks.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>
#include <QtEndian>
#include <QCoreApplication>
#include <QEventLoop>

namespace modbus::session {

namespace {

QString trClient(const char* text)
{
    return QCoreApplication::translate("ModbusClient", text);
}

QString exceptionName(base::ExceptionCode code)
{
    using base::ExceptionCode;
    switch (code) {
    case ExceptionCode::IllegalFunction:
        return trClient("Illegal Function");
    case ExceptionCode::IllegalDataAddress:
        return trClient("Illegal Data Address");
    case ExceptionCode::IllegalDataValue:
        return trClient("Illegal Data Value");
    case ExceptionCode::ServerDeviceFailure:
        return trClient("Server Device Failure");
    case ExceptionCode::Acknowledge:
        return trClient("Acknowledge");
    case ExceptionCode::ServerDeviceBusy:
        return trClient("Server Device Busy");
    case ExceptionCode::NegativeAcknowledge:
        return trClient("Negative Acknowledge");
    case ExceptionCode::MemoryParityError:
        return trClient("Memory Parity Error");
    case ExceptionCode::GatewayPathUnavailable:
        return trClient("Gateway Path Unavailable");
    case ExceptionCode::GatewayTargetDeviceFailed:
        return trClient("Gateway Target Device Failed To Respond");
    default:
        return trClient("Unknown Exception");
    }
}

QString buildModbusExceptionMessage(int slaveId, base::FunctionCode requestFc, base::ExceptionCode exceptionCode)
{
    return trClient("Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)")
        .arg(slaveId)
        .arg(static_cast<int>(requestFc), 2, 16, QChar('0'))
        .arg(static_cast<int>(exceptionCode), 2, 16, QChar('0'))
        .arg(exceptionName(exceptionCode));
}

double stopBitsToCount(int stopBits)
{
    switch (stopBits) {
    case 2:
        return 2.0;
    case 3:
        return 1.5;
    default:
        return 1.0;
    }
}

int parityBitCount(int parity)
{
    return parity == 0 ? 0 : 1;
}

int bitsPerCharacter(const base::ModbusConfig& config)
{
    // Modbus RTU specification (Modbus over Serial Line V1.02, section 2.5.1):
    // "The format (11 bits) for each byte in RTU mode is: 1 start bit, 8 data bits,
    // 1 parity bit, 1 stop bit. If No Parity is implemented, an additional stop bit 
    // is transmitted to fill out the character frame to a full 11-bit asynchronous character."
    // 
    // However, in the real world, the vast majority of devices default to 8N1 (10 bits),
    // which technically violates the 11-bit rule but is the de facto industry standard.
    // We calculate timing based on the *actual* configured bits.
    const int startBits = 1;
    return static_cast<int>(std::ceil(static_cast<double>(startBits) + static_cast<double>(config.dataBits) +
                                      static_cast<double>(parityBitCount(config.parity)) +
                                      stopBitsToCount(config.stopBits)));
}

std::chrono::microseconds calculateRtuInterFrameDelay(const base::ModbusConfig& config)
{
    if (config.interFrameDelayUs > 0) {
        return std::chrono::microseconds(config.interFrameDelayUs);
    }
    if (config.baudRate <= 0) {
        return std::chrono::microseconds(1750);
    }
    if (config.baudRate > 19200) {
        return std::chrono::microseconds(1750);
    }

    const double charTimeUs =
        (static_cast<double>(bitsPerCharacter(config)) * 1000000.0) / static_cast<double>(config.baudRate);
    return std::chrono::microseconds(static_cast<long long>(std::ceil(3.5 * charTimeUs)));
}

std::chrono::microseconds calculateRtuInterCharacterDelay(const base::ModbusConfig& config)
{
    if (config.baudRate <= 0) {
        return std::chrono::microseconds(750);
    }
    if (config.baudRate > 19200) {
        return std::chrono::microseconds(750);
    }

    const double charTimeUs =
        (static_cast<double>(bitsPerCharacter(config)) * 1000000.0) / static_cast<double>(config.baudRate);
    return std::chrono::microseconds(static_cast<long long>(std::ceil(1.5 * charTimeUs)));
}

std::chrono::microseconds calculateRtuFrameDuration(const base::ModbusConfig& config, qsizetype frameBytes)
{
    if (config.baudRate <= 0 || frameBytes <= 0) {
        return std::chrono::microseconds::zero();
    }

    const double totalBits = static_cast<double>(bitsPerCharacter(config)) * static_cast<double>(frameBytes);
    const double frameUs = (totalBits * 1000000.0) / static_cast<double>(config.baudRate);
    return std::chrono::microseconds(static_cast<long long>(std::ceil(frameUs)));
}

bool isBroadcastWriteFunction(base::FunctionCode functionCode)
{
    using base::FunctionCode;
    switch (functionCode) {
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        return true;
    default:
        return false;
    }
}

bool readBigEndianUInt16(const QByteArray& data, qsizetype offset, uint16_t& value)
{
    if (offset < 0 || offset + 2 > data.size()) {
        return false;
    }
    value = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.constData() + offset));
    return true;
}

bool isAddressRangeValid(uint16_t startAddress, uint16_t quantity)
{
    if (quantity == 0) {
        return false;
    }
    const uint32_t endAddress = static_cast<uint32_t>(startAddress) + static_cast<uint32_t>(quantity) - 1U;
    return endAddress <= static_cast<uint32_t>(app::constants::Values::Modbus::kMaxAddress);
}

int sanitizeDelayMs(int value)
{
    return std::max(0, value);
}

int calculateBackoffDelayMs(const base::ModbusConfig& config, int baseDelayMs, int maxDelayMs, int attempt)
{
    const double factor = std::max(1.0, config.retryBackoffFactor);
    const int sanitizedBase = sanitizeDelayMs(baseDelayMs);
    const int sanitizedMax = std::max(sanitizedBase, sanitizeDelayMs(maxDelayMs));
    const double scaled = static_cast<double>(sanitizedBase) * std::pow(factor, static_cast<double>(attempt));
    const int cappedDelay = std::min(sanitizedMax, static_cast<int>(std::llround(scaled)));

    const int jitterPercent = std::clamp(config.retryJitterPercent, 0, 100);
    if (jitterPercent == 0 || cappedDelay == 0) {
        return cappedDelay;
    }

    thread_local std::mt19937 generator{std::random_device{}()};
    const int jitterWindow = std::max(1, (cappedDelay * jitterPercent) / 100);
    std::uniform_int_distribution<int> distribution(-jitterWindow, jitterWindow);
    return std::max(0, cappedDelay + distribution(generator));
}

constexpr auto kQtWaitSlice = std::chrono::milliseconds(5);

void pumpCurrentThreadEvents(std::chrono::milliseconds maxTime)
{
    if (!QCoreApplication::instance()) {
        return;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, static_cast<int>(maxTime.count()));
}

} // namespace

const char* ModbusClient::toString(ConnectionState state) {
    switch (state) {
    case ConnectionState::Disconnected: return "Disconnected";
    case ConnectionState::Connecting: return "Connecting";
    case ConnectionState::Connected: return "Connected";
    case ConnectionState::Reconnecting: return "Reconnecting";
    case ConnectionState::Disconnecting: return "Disconnecting";
    case ConnectionState::Failed: return "Failed";
    }
    return "Unknown";
}

const char* ModbusClient::toString(RequestState state) {
    switch (state) {
    case RequestState::Idle: return "Idle";
    case RequestState::Sending: return "Sending";
    case RequestState::Waiting: return "Waiting";
    case RequestState::Completed: return "Completed";
    case RequestState::Failed: return "Failed";
    case RequestState::Aborted: return "Aborted";
    }
    return "Unknown";
}

void ModbusClient::transitionConnectionState(ConnectionState newState, const char* reason) {
    ConnectionState oldState = connectionState_.exchange(newState);
    if (oldState == newState) {
        return;
    }
    spdlog::info("ModbusClient: connection {} -> {} ({})",
                 toString(oldState),
                 toString(newState),
                 reason);
}

void ModbusClient::transitionTo(RequestState newState, const char* reason) {
    RequestState oldState = requestState_.exchange(newState);
    if (oldState == newState) {
        return;
    }
    spdlog::info("ModbusClient: state {} -> {} ({})",
                 toString(oldState),
                 toString(newState),
                 reason);
}

ModbusClient::ConnectionState ModbusClient::connectionState() const {
    return connectionState_.load();
}

ModbusClient::RequestState ModbusClient::requestState() const {
    return requestState_.load();
}

int ModbusClient::enqueuePendingRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    PendingRequest item;
    item.requestId = nextRequestId_++;
    item.functionCode = request.functionCode();
    item.slaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    item.timeoutMs = config_.timeoutMs;
    item.retries = config_.retries;
    item.enqueueAt = std::chrono::steady_clock::now();
    pendingRequests_.push_back(item);
    spdlog::info("ModbusClient: enqueue request id={}, fc={}, slave={}, queue={}",
                 item.requestId,
                 static_cast<int>(item.functionCode),
                 item.slaveId,
                 pendingRequests_.size());
    return item.requestId;
}

void ModbusClient::finishPendingRequest(int requestId, bool success, const QString& error) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    auto it = std::find_if(pendingRequests_.begin(), pendingRequests_.end(), [requestId](const PendingRequest& item) {
        return item.requestId == requestId;
    });
    if (it == pendingRequests_.end()) {
        spdlog::warn("ModbusClient: request id={} not found in queue", requestId);
        return;
    }
    const auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - it->enqueueAt).count();
    spdlog::info("ModbusClient: finish request id={}, success={}, queue_wait={}ms, error='{}'",
                 requestId,
                 success,
                 waitMs,
                 error.toStdString());
    pendingRequests_.erase(it);
}

void ModbusClient::clearRuntimeState(bool clearPendingQueue) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
    completedRtuFrames_.clear();
    responseReady_ = false;
    writeDrained_ = true;
    lastChannelError_.clear();
    lastRtuByteReceivedAt_ = std::chrono::steady_clock::time_point{};
    lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    nextRtuSendAllowedAt_ = std::chrono::steady_clock::time_point{};
    if (clearPendingQueue) {
        std::lock_guard<std::mutex> pendingLock(pendingMutex_);
        pendingRequests_.clear();
    }
    if (transport_) {
        transport_->resetPendingState();
    }
}

ModbusClient::ModbusClient(std::shared_ptr<io::IChannel> channel, 
                           std::shared_ptr<transport::ITransport> transport)
    : channel_(std::move(channel)), transport_(std::move(transport)) {
    
    // 设置 Channel 回调
    channel_->setReadHandler([this](QByteArrayView data) {
        onDataReceived(data);
    });
    
    channel_->setErrorHandler([this](const QString& error) {
        onChannelError(error);
    });

    channel_->setWriteDrainedHandler([this]() {
        std::lock_guard<std::mutex> lock(mutex_);
        writeDrained_ = true;
        lastWriteDrainedAt_ = std::chrono::steady_clock::now();
        cv_.notify_one();
    });
    stateHandlerId_ = channel_->addStateHandler([this](io::ChannelState) {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    });
}

ModbusClient::~ModbusClient() {
    abort();
    if (channel_) {
        channel_->setReadHandler({});
        channel_->setErrorHandler({});
        channel_->setWriteDrainedHandler({});
        channel_->removeStateHandler(stateHandlerId_);
    }
    clearRuntimeState(true);
}

bool ModbusClient::connect() {
    if (!channel_) return false;

    aborted_ = false;
    const bool connected = ensureConnected(config_.autoReconnect);
    if (connected) {
        clearRuntimeState(false);
        transitionTo(RequestState::Idle, "connect");
    }
    return connected;
}

void ModbusClient::disconnect() {
    aborted_ = true;
    transitionConnectionState(ConnectionState::Disconnecting, "disconnect");
    if (channel_) {
        channel_->close();
    }
    clearRuntimeState(true);
    aborted_ = false;
    transitionConnectionState(ConnectionState::Disconnected, "disconnect-complete");
    transitionTo(RequestState::Idle, "disconnect");
}

void ModbusClient::abort() {
    spdlog::info("ModbusClient: Abort requested");
    aborted_ = true;
    transitionTo(RequestState::Aborted, "abort");
    cv_.notify_all();
}

bool ModbusClient::isConnected() const {
    return channel_ && channel_->isOpen();
}

QString ModbusClient::lastChannelError() {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastChannelError_;
}

void ModbusClient::setConfig(const base::ModbusConfig& config) {
    config_ = config;
    if (isConnected()) {
        // 如果已经连接，动态更新超时
        io::Timeouts timeouts;
        timeouts.readMs = config_.timeoutMs;
        timeouts.writeMs = config_.timeoutMs;
        channel_->setTimeouts(timeouts);
    }
}

bool ModbusClient::ensureConnected(bool allowReconnect) {
    if (!channel_) {
        std::lock_guard<std::mutex> lock(mutex_);
        lastChannelError_ = trClient("No channel attached");
        transitionConnectionState(ConnectionState::Failed, "no-channel");
        return false;
    }

    io::Timeouts timeouts;
    timeouts.readMs = config_.timeoutMs;
    timeouts.writeMs = config_.timeoutMs;
    channel_->setTimeouts(timeouts);

    if (channel_->isOpen()) {
        transitionConnectionState(ConnectionState::Connected, "already-open");
        return true;
    }

    const int attempts = allowReconnect ? std::max(1, config_.retries + 1) : 1;
    QString connectError;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        if (aborted_.load()) {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_ = trClient("Aborted");
            transitionConnectionState(ConnectionState::Failed, "connect-aborted");
            return false;
        }

        transitionConnectionState(attempt == 0 ? ConnectionState::Connecting : ConnectionState::Reconnecting,
                                  attempt == 0 ? "connect-attempt" : "reconnect-attempt");
        {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_.clear();
            responseReady_ = false;
        }

        if (!channel_->open()) {
            connectError = trClient("Failed to dispatch channel open");
        } else {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(config_.timeoutMs);
            if (waitForChannelState(io::ChannelState::Open, deadline, &connectError)) {
                transitionConnectionState(ConnectionState::Connected, "connect-success");
                return true;
            }
        }

        channel_->close();
        transitionConnectionState(ConnectionState::Failed, "connect-failed");
        if (attempt + 1 >= attempts) {
            break;
        }

        const int reconnectDelayMs = calculateBackoffDelayMs(
            config_,
            config_.reconnectBaseMs,
            config_.reconnectMaxMs,
            attempt);
        if (!waitForAbortableDelay(std::chrono::milliseconds(reconnectDelayMs))) {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_ = trClient("Aborted");
            transitionConnectionState(ConnectionState::Failed, "reconnect-aborted");
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    lastChannelError_ = connectError.isEmpty() ? trClient("Connect timeout") : connectError;
    spdlog::warn("ModbusClient: connect failed target={}:{} reason={} channelState={}",
                 config_.ipAddress.toStdString(),
                 config_.port,
                 lastChannelError_.toStdString(),
                 static_cast<int>(channel_->state()));
    return false;
}

bool ModbusClient::waitForChannelState(io::ChannelState expectedState,
                                       std::chrono::steady_clock::time_point deadline,
                                       QString* errorOut) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            if (errorOut) {
                *errorOut = trClient("Aborted");
            }
            return false;
        }
        if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
            return true;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!lastChannelError_.isEmpty()) {
                if (errorOut) {
                    *errorOut = lastChannelError_;
                }
                lastChannelError_.clear();
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            if (errorOut && errorOut->isEmpty()) {
                *errorOut = trClient("Channel entered error state");
            }
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, slice, [this, expectedState]() {
                return aborted_.load() ||
                    !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error ||
                    channel_->state() == expectedState ||
                    (expectedState == io::ChannelState::Open && channel_->isOpen());
            })) {
            continue;
        }
    }

    if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
        return true;
    }
    if (errorOut && errorOut->isEmpty()) {
        *errorOut = trClient("Connect timeout");
    }
    return false;
}

bool ModbusClient::waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                                     std::chrono::steady_clock::time_point* drainedAt) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (writeDrained_) {
                if (drainedAt) {
                    *drainedAt = (lastWriteDrainedAt_ == std::chrono::steady_clock::time_point{})
                        ? std::chrono::steady_clock::now()
                        : lastWriteDrainedAt_;
                }
                return true;
            }
            if (!lastChannelError_.isEmpty()) {
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, slice, [this]() {
                return aborted_.load() || writeDrained_ || !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error;
            })) {
            continue;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (writeDrained_) {
        if (drainedAt) {
            *drainedAt = (lastWriteDrainedAt_ == std::chrono::steady_clock::time_point{})
                ? std::chrono::steady_clock::now()
                : lastWriteDrainedAt_;
        }
        return true;
    }
    return false;
}

bool ModbusClient::waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline) {
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (aborted_.load() || responseReady_ || !lastChannelError_.isEmpty() ||
                channel_->state() == io::ChannelState::Error) {
                return true;
            }
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, slice, [this]() {
                return aborted_.load() || responseReady_ || !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error;
            })) {
            return true;
        }
    }
    return false;
}

ModbusResponse ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    aborted_ = false;

    const base::ModbusConfig activeConfig = config_;
    const int retries = std::max(0, activeConfig.retries);
    ModbusResponse lastResponse = ModbusResponse::Error(trClient("Unknown error"));
    const int requestId = enqueuePendingRequest(request, slaveId);
    transitionTo(RequestState::Idle, "request-start");

    for (int i = 0; i <= retries; ++i) {
        if (aborted_) {
            transitionTo(RequestState::Aborted, "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error(trClient("Aborted"));
        }

        lastResponse = sendRequestInternal(request, slaveId);
        if (lastResponse.isSuccess) {
            transitionTo(RequestState::Completed, "request-success");
            finishPendingRequest(requestId, true, QString());
            return lastResponse;
        }
        
        if (i < retries && !aborted_) {
            transitionTo(RequestState::Failed, "request-retry");
            const int retryDelayMs = calculateBackoffDelayMs(
                activeConfig,
                activeConfig.retryIntervalMs,
                activeConfig.maxRetryIntervalMs,
                i);
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}", 
                         i + 1,
                         retries,
                         lastResponse.error.toStdString());
            if (!waitForAbortableDelay(std::chrono::milliseconds(retryDelayMs))) {
                transitionTo(RequestState::Aborted, "request-aborted-during-backoff");
                finishPendingRequest(requestId, false, "Aborted");
                return ModbusResponse::Error(trClient("Aborted"));
            }
        }
    }
    if (aborted_) {
        transitionTo(RequestState::Aborted, "request-aborted-after-retries");
    } else {
        transitionTo(RequestState::Failed, "request-failed");
    }
    finishPendingRequest(requestId, false, lastResponse.error);
    return lastResponse;
}

void ModbusClient::sendRaw(const QByteArray& data) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    if (isConnected()) {
        waitForRtuInterFrameDelay();
        {
            std::lock_guard<std::mutex> stateLock(mutex_);
            lastChannelError_.clear();
            writeDrained_ = config_.mode != base::ModbusMode::RTU;
            lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
        }
        if (channel_->write(data)) {
            updateRtuSendWindow(data.size());
            if (config_.mode == base::ModbusMode::RTU) {
                const auto writeDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(config_.timeoutMs);
                waitForWriteDrain(writeDeadline, nullptr);
            }
        }
    }
}

ModbusResponse ModbusClient::sendRequestInternal(const base::Pdu& request, int slaveId) {
    if (!ensureConnected(config_.autoReconnect)) {
        transitionTo(RequestState::Failed, "not-connected");
        std::lock_guard<std::mutex> lock(mutex_);
        return ModbusResponse::Error(lastChannelError_.isEmpty() ? trClient("Not connected") : lastChannelError_);
    }

    const QString validationError = validateRequest(request, slaveId);
    if (!validationError.isEmpty()) {
        transitionTo(RequestState::Failed, "request-validation-failed");
        return ModbusResponse::Error(validationError);
    }

    // 1. 清理旧缓冲区和状态
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (aborted_) {
            transitionTo(RequestState::Aborted, "aborted-before-build");
            return ModbusResponse::Error(trClient("Aborted"));
        }
        buffer_.clear();
        completedRtuFrames_.clear();
        responseReady_ = false;
        writeDrained_ = config_.mode != base::ModbusMode::RTU;
        lastChannelError_.clear();
        lastRtuByteReceivedAt_ = std::chrono::steady_clock::time_point{};
        lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    }
    transport_->resetPendingState();

    // 2. 构建 ADU
    int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    if (isRtuBroadcastRequest(targetSlaveId, request.functionCode()) &&
        !isBroadcastWriteFunction(request.functionCode())) {
        transitionTo(RequestState::Failed, "invalid-rtu-broadcast-function");
        return ModbusResponse::Error(trClient("RTU broadcast only supports write function codes"));
    }
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);
    waitForRtuInterFrameDelay();

    // 3. 发送数据
    if (!channel_->write(adu)) {
        transitionTo(RequestState::Failed, "write-failed");
        return ModbusResponse::Error(trClient("Write failed"));
    }
    updateRtuSendWindow(adu.size());
    transitionTo(RequestState::Sending, "write-success");
    auto start = std::chrono::steady_clock::now();

    if (config_.mode == base::ModbusMode::RTU) {
        std::chrono::steady_clock::time_point drainedAt{};
        const auto writeDeadline = start + std::chrono::milliseconds(config_.timeoutMs);
        if (!waitForWriteDrain(writeDeadline, &drainedAt)) {
            transitionTo(RequestState::Failed, "write-drain-timeout");
            std::lock_guard<std::mutex> lock(mutex_);
            const QString error = lastChannelError_.isEmpty()
                ? trClient("Write drain timeout")
                : lastChannelError_;
            lastChannelError_.clear();
            return ModbusResponse::Error(error);
        }
        if (drainedAt != std::chrono::steady_clock::time_point{}) {
            start = drainedAt;
        }
    }

    if (!shouldWaitForResponse(targetSlaveId, request.functionCode())) {
        transitionTo(RequestState::Completed, "broadcast-write-no-response");
        return ModbusResponse::NoResponseExpected(base::Pdu(request.functionCode(), request.data()));
    }

    // 4. 等待响应
    // 使用绝对时间作为截止日期，避免循环中相对时间重置
    auto deadline = start + std::chrono::milliseconds(config_.timeoutMs);
    if (config_.mode == base::ModbusMode::RTU) {
        deadline += calculateRtuInterFrameDelay(config_);
    }
    transitionTo(RequestState::Waiting, "wait-response");
    int droppedInvalidBytes = 0;
    spdlog::info("ModbusClient: Entering wait loop, deadline in {}ms", config_.timeoutMs);
    
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
        const bool rtuFrameReady = isRtuFrameReadyToParseLocked(now);
        if (!rtuFrameReady && !responseReady_ && lastChannelError_.isEmpty() && !aborted_.load()) {
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(deadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                spdlog::warn("ModbusClient: request timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
                return ModbusResponse::Error(trClient("Timeout"));
            }
            continue;
        }
        if (!rtuFrameReady && config_.mode == base::ModbusMode::RTU && !buffer_.isEmpty() &&
            lastRtuByteReceivedAt_ != std::chrono::steady_clock::time_point{}) {
            responseReady_ = false;
            const auto frameDeadline = std::min(deadline, nextRtuFrameBoundaryLocked());
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(frameDeadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                spdlog::warn("ModbusClient: RTU frame wait timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
                return ModbusResponse::Error(trClient("Timeout"));
            }
            continue;
        }
        if (aborted_) {
            spdlog::info("ModbusClient: Aborted during wait");
            transitionTo(RequestState::Aborted, "aborted-during-wait");
            return ModbusResponse::Error(trClient("Aborted"));
        }
        if (!lastChannelError_.isEmpty()) {
            QString err = lastChannelError_;
            lastChannelError_.clear();
            transitionTo(RequestState::Failed, "channel-error");
            return ModbusResponse::Error(err);
        }

        responseReady_ = false;

        while (true) {
            // 对于 RTU，走基于静默时间的帧提取流程
            if (config_.mode == base::ModbusMode::RTU) {
                QByteArray frame;
                if (tryPopRtuResponseFrameLocked(frame)) {
                    lock.unlock();
                    auto parseResult = transport_->parseResponse(frame);
                    if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                        const auto& pdu = *parseResult.pdu;
                        if (pdu.isException()) {
                            transitionTo(RequestState::Failed, "modbus-exception");
                            const QString exceptionMessage = buildModbusExceptionMessage(
                                slaveId,
                                request.functionCode(),
                                pdu.exceptionCode());
                            spdlog::warn("ModbusClient: {}", exceptionMessage.toStdString());
                            return ModbusResponse::Error(exceptionMessage);
                        }
                        if (pdu.originalFunctionCode() != request.functionCode()) {
                            lock.lock();
                            continue;
                        }
                        const QString responseValidationError = base::validateResponsePdu(request, pdu);
                        if (!responseValidationError.isEmpty()) {
                            transitionTo(RequestState::Failed, "response-validation-failed");
                            return ModbusResponse::Error(responseValidationError);
                        }
                        auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start).count();
                        transitionTo(RequestState::Completed, "response-parsed");
                        return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                    }
                    if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                        lock.lock();
                        continue;
                    }
                    transitionTo(RequestState::Failed, "response-parse-failed");
                    return ModbusResponse::Error(trClient("Response parsing failed"));
                }
                break;
            }

            int integrity = transport_->checkIntegrity(buffer_);
            if (integrity > 0) {
                QByteArray frame = buffer_.left(integrity);
                buffer_.remove(0, integrity);
                lock.unlock();
                auto parseResult = transport_->parseResponse(frame);
                if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                    const auto& pdu = *parseResult.pdu;
                    if (pdu.isException()) {
                        transitionTo(RequestState::Failed, "modbus-exception");
                        const QString exceptionMessage = buildModbusExceptionMessage(
                            slaveId,
                            request.functionCode(),
                            pdu.exceptionCode());
                        spdlog::warn("ModbusClient: {}", exceptionMessage.toStdString());
                        return ModbusResponse::Error(exceptionMessage);
                    }
                    if (pdu.originalFunctionCode() != request.functionCode()) {
                        lock.lock();
                        continue;
                    }
                    const QString responseValidationError = base::validateResponsePdu(request, pdu);
                    if (!responseValidationError.isEmpty()) {
                        transitionTo(RequestState::Failed, "response-validation-failed");
                        return ModbusResponse::Error(responseValidationError);
                    }
                    auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                    transitionTo(RequestState::Completed, "response-parsed");
                    return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                }
                if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                    lock.lock();
                    continue;
                }
                if (parseResult.status == transport::ParseResponseStatus::Invalid) {
                    transitionTo(RequestState::Failed, "response-parse-failed");
                    return ModbusResponse::Error(trClient("Response parsing failed"));
                }
                lock.lock();
                continue;
            } else if (integrity == -1) {
                if (!buffer_.isEmpty()) {
                    spdlog::warn("ModbusClient: RTU CRC mismatch, dropping first byte [{}] of buffer={} totalDropped={}",
                                 static_cast<int>(buffer_.at(0)),
                                 QString(buffer_.toHex(' ').toUpper()).toStdString(),
                                 droppedInvalidBytes + 1);
                    buffer_.remove(0, 1);
                    ++droppedInvalidBytes;
                }
                if (droppedInvalidBytes > app::constants::Values::Modbus::kMaxDroppedInvalidBytes) {
                    spdlog::error("ModbusClient: RTU CRC mismatch limit exceeded ({}), aborting request", droppedInvalidBytes);
                    transitionTo(RequestState::Failed, "too-many-invalid-bytes");
                    return ModbusResponse::Error(trClient("Too many invalid response bytes"));
                }
                responseReady_ = !buffer_.isEmpty();
                continue;
            }
            if (config_.mode == base::ModbusMode::RTU && !buffer_.isEmpty() &&
                isRtuFrameReadyToParseLocked(std::chrono::steady_clock::now())) {
                transitionTo(RequestState::Failed, "incomplete-rtu-frame-after-gap");
                return ModbusResponse::Error(trClient("Incomplete RTU frame after inter-frame silence"));
            }
            break;
        }
        
        if (std::chrono::steady_clock::now() >= deadline) {
            transitionTo(RequestState::Failed, "timeout-full-packet");
            spdlog::warn("ModbusClient: full packet wait timeout slave={} fc={} timeoutMs={}",
                         slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
            return ModbusResponse::Error(trClient("Timeout while waiting for full packet"));
        }
    }
}

bool ModbusClient::isRtuBroadcastRequest(int slaveId, base::FunctionCode functionCode) const {
    Q_UNUSED(functionCode);
    return config_.mode == base::ModbusMode::RTU && slaveId == 0;
}

bool ModbusClient::shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const {
    if (!isRtuBroadcastRequest(slaveId, functionCode)) {
        return true;
    }
    return false;
}

QString ModbusClient::validateRequest(const base::Pdu& request, int slaveId) const {
    const int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    if (config_.mode == base::ModbusMode::RTU &&
        (targetSlaveId < app::constants::Values::Modbus::kMinSlaveId ||
         targetSlaveId > app::constants::Values::Modbus::kMaxSlaveId)) {
        return QString("Invalid RTU slave id: %1").arg(targetSlaveId);
    }

    const QByteArray data = request.data();
    if (data.size() > app::constants::Values::Modbus::kMaxPduDataLength) {
        return QString("PDU data too long: %1 bytes").arg(data.size());
    }

    uint16_t quantity = 0;
    uint16_t startAddress = 0;
    uint16_t value = 0;
    uint8_t declaredByteCount = 0;
    switch (request.functionCode()) {
    case base::FunctionCode::ReadCoils:
    case base::FunctionCode::ReadDiscreteInputs:
    case base::FunctionCode::ReadHoldingRegisters:
    case base::FunctionCode::ReadInputRegisters:
        if (data.size() != 4 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, quantity)) {
            return QString("Invalid read request payload length: %1").arg(data.size());
        }
        if (quantity < app::constants::Values::Modbus::kMinQuantity) {
            return QString("Read quantity must be at least %1").arg(app::constants::Values::Modbus::kMinQuantity);
        }
        if (!isAddressRangeValid(startAddress, quantity)) {
            return QString("Read address range exceeds limit: start=%1 quantity=%2")
                .arg(startAddress)
                .arg(quantity);
        }
        if ((request.functionCode() == base::FunctionCode::ReadCoils ||
             request.functionCode() == base::FunctionCode::ReadDiscreteInputs) &&
            quantity > app::constants::Values::Modbus::kMaxReadBitsQuantity) {
            return QString("Read bit quantity exceeds limit: %1").arg(quantity);
        }
        if ((request.functionCode() == base::FunctionCode::ReadHoldingRegisters ||
             request.functionCode() == base::FunctionCode::ReadInputRegisters) &&
            quantity > app::constants::Values::Modbus::kMaxReadQuantity) {
            return QString("Read register quantity exceeds limit: %1").arg(quantity);
        }
        break;
    case base::FunctionCode::WriteSingleCoil:
    case base::FunctionCode::WriteSingleRegister:
        if (data.size() != 4 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, value)) {
            return QString("Invalid write-single request payload length: %1").arg(data.size());
        }
        if (!isAddressRangeValid(startAddress, 1)) {
            return QString("Write address out of range: %1").arg(startAddress);
        }
        if (request.functionCode() == base::FunctionCode::WriteSingleCoil &&
            value != 0x0000 && value != 0xFF00) {
            return QString("Invalid single coil value: 0x%1")
                .arg(value, 4, 16, QChar('0'))
                .toUpper();
        }
        break;
    case base::FunctionCode::WriteMultipleCoils:
    case base::FunctionCode::WriteMultipleRegisters:
        if (data.size() < 5 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, quantity)) {
            return QString("Invalid write-multiple request payload length: %1").arg(data.size());
        }
        declaredByteCount = static_cast<uint8_t>(data[4]);
        if (quantity < app::constants::Values::Modbus::kMinQuantity) {
            return QString("Write quantity must be at least %1").arg(app::constants::Values::Modbus::kMinQuantity);
        }
        if (!isAddressRangeValid(startAddress, quantity)) {
            return QString("Write address range exceeds limit: start=%1 quantity=%2")
                .arg(startAddress)
                .arg(quantity);
        }
        if (static_cast<int>(declaredByteCount) != data.size() - 5) {
            return QString("Write request byte count mismatch: declared %1, actual %2")
                .arg(declaredByteCount)
                .arg(data.size() - 5);
        }
        if (request.functionCode() == base::FunctionCode::WriteMultipleCoils) {
            if (quantity > app::constants::Values::Modbus::kMaxWriteCoilsQuantity) {
                return QString("Write coil quantity exceeds limit: %1").arg(quantity);
            }
            const int expectedByteCount = (static_cast<int>(quantity) + 7) / 8;
            if (declaredByteCount != expectedByteCount) {
                return QString("Write coil byte count mismatch: declared %1, expected %2")
                    .arg(declaredByteCount)
                    .arg(expectedByteCount);
            }
        } else {
            if (quantity > app::constants::Values::Modbus::kMaxWriteRegistersQuantity) {
                return QString("Write register quantity exceeds limit: %1").arg(quantity);
            }
            const int expectedByteCount = static_cast<int>(quantity) * 2;
            if (declaredByteCount != expectedByteCount) {
                return QString("Write register byte count mismatch: declared %1, expected %2")
                    .arg(declaredByteCount)
                    .arg(expectedByteCount);
            }
        }
        break;
    default:
        return QString("Unsupported function code: 0x%1")
            .arg(static_cast<int>(request.functionCode()), 2, 16, QChar('0'))
            .toUpper();
    }

    const int tcpMbapLength = 2 + data.size();
    if (tcpMbapLength > app::constants::Values::Modbus::kMaxTcpMbapLength) {
        return QString("TCP MBAP length exceeds limit: %1").arg(tcpMbapLength);
    }

    const int rtuAduLength = 4 + data.size();
    if (rtuAduLength > 256) {
        return QString("RTU ADU length exceeds limit: %1").arg(rtuAduLength);
    }

    return QString();
}

void ModbusClient::waitForRtuInterFrameDelay() {
    if (config_.mode != base::ModbusMode::RTU) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (nextRtuSendAllowedAt_ > now) {
        waitForAbortableDelay(nextRtuSendAllowedAt_ - now);
    }
}

bool ModbusClient::waitForAbortableDelay(std::chrono::steady_clock::duration delay) {
    if (delay <= std::chrono::steady_clock::duration::zero()) {
        return !aborted_.load();
    }

    const auto deadline = std::chrono::steady_clock::now() + delay;
    while (!aborted_.load() && std::chrono::steady_clock::now() < deadline) {
        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, slice, [this]() {
            return aborted_.load();
        });
    }
    return !aborted_.load();
}

void ModbusClient::updateRtuSendWindow(qsizetype frameBytes) {
    if (config_.mode != base::ModbusMode::RTU) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    nextRtuSendAllowedAt_ = now + calculateRtuFrameDuration(config_, frameBytes) + calculateRtuInterFrameDelay(config_);
}

bool ModbusClient::isRtuFrameReadyToParseLocked(std::chrono::steady_clock::time_point now) const {
    if (config_.mode != base::ModbusMode::RTU) {
        return false;
    }
    if (!completedRtuFrames_.empty()) {
        return true;
    }
    if (buffer_.isEmpty()) {
        return false;
    }
    if (lastRtuByteReceivedAt_ == std::chrono::steady_clock::time_point{}) {
        return false;
    }
    return now >= nextRtuFrameBoundaryLocked();
}

std::chrono::steady_clock::time_point ModbusClient::nextRtuFrameBoundaryLocked() const {
    if (lastRtuByteReceivedAt_ == std::chrono::steady_clock::time_point{}) {
        return std::chrono::steady_clock::time_point{};
    }
    return lastRtuByteReceivedAt_ + calculateRtuInterFrameDelay(config_);
}

bool ModbusClient::tryPopRtuResponseFrameLocked(QByteArray& frame) {
    if (config_.mode != base::ModbusMode::RTU) {
        return false;
    }

    if (!completedRtuFrames_.empty()) {
        frame = completedRtuFrames_.front();
        completedRtuFrames_.pop_front();
        return true;
    }

    if (buffer_.isEmpty() || buffer_.size() < 5 || !isRtuFrameReadyToParseLocked(std::chrono::steady_clock::now())) {
        return false;
    }

    frame = std::move(buffer_);
    buffer_.clear();
    lastRtuByteReceivedAt_ = std::chrono::steady_clock::time_point{};
    return true;
}

void ModbusClient::onDataReceived(QByteArrayView data) {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::info("ModbusClient: Data received, size={}, notifying loop", data.size());
    if (config_.mode == base::ModbusMode::RTU) {
        const auto now = std::chrono::steady_clock::now();
        if (!buffer_.isEmpty() && lastRtuByteReceivedAt_ != std::chrono::steady_clock::time_point{}) {
            const auto silentInterval = now - lastRtuByteReceivedAt_;
            if (silentInterval >= calculateRtuInterFrameDelay(config_)) {
                completedRtuFrames_.push_back(buffer_);
                buffer_.clear();
            } else if (silentInterval > calculateRtuInterCharacterDelay(config_)) {
                spdlog::warn("ModbusClient: discarding RTU fragment after inter-character gap violation");
                buffer_.clear();
            }
        }
        buffer_.append(data);
        if (buffer_.size() > app::constants::Values::Modbus::kMaxRtuBufferedBytes) {
            spdlog::warn("ModbusClient: RTU buffer exceeded {} bytes limit, clearing",
                         app::constants::Values::Modbus::kMaxRtuBufferedBytes);
            buffer_.clear();
        }
        lastRtuByteReceivedAt_ = now;
    } else {
        buffer_.append(data);
        if (buffer_.size() > app::constants::Values::Modbus::kMaxTcpBufferedBytes) {
            spdlog::warn("ModbusClient: TCP buffer exceeded {} bytes limit, dropping oldest bytes",
                         app::constants::Values::Modbus::kMaxTcpBufferedBytes);
            buffer_.remove(0, buffer_.size() - app::constants::Values::Modbus::kMaxTcpBufferedBytes);
        }
    }
    responseReady_ = true; // 即使不完整也唤醒，由工作线程检查完整性
    cv_.notify_one();
}

void ModbusClient::onChannelError(const QString& error) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastChannelError_ = error;
    }
    spdlog::warn("ModbusClient: channel error forwarded: '{}' state={} connState={}",
                 error.toStdString(),
                 toString(requestState_.load()),
                 toString(connectionState_.load()));
    cv_.notify_one();
}

} // namespace modbus::session
