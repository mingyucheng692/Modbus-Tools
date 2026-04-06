#include "ModbusClient.h"
#include "AppConstants.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>
#include <QtEndian>

namespace modbus::session {

namespace {

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
    return static_cast<int>(std::ceil(1.0 + static_cast<double>(config.dataBits) +
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
    return endAddress <= static_cast<uint32_t>(app::constants::Constants::Modbus::kMaxAddress);
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
    responseReady_ = false;
    writeDrained_ = true;
    lastError_.clear();
    lastRtuByteReceivedAt_ = std::chrono::steady_clock::time_point{};
    lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    nextRtuSendAllowedAt_ = std::chrono::steady_clock::time_point{};
    if (clearPendingQueue) {
        std::lock_guard<std::mutex> pendingLock(pendingMutex_);
        pendingRequests_.clear();
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
        onError(error);
    });

    channel_->setWriteDrainedHandler([this]() {
        std::lock_guard<std::mutex> lock(mutex_);
        writeDrained_ = true;
        lastWriteDrainedAt_ = std::chrono::steady_clock::now();
        cv_.notify_one();
    });
    channel_->setStateHandler([this](io::ChannelState) {
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
        channel_->setStateHandler({});
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
        lastError_ = QStringLiteral("No channel attached");
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
            lastError_ = QStringLiteral("Aborted");
            transitionConnectionState(ConnectionState::Failed, "connect-aborted");
            return false;
        }

        transitionConnectionState(attempt == 0 ? ConnectionState::Connecting : ConnectionState::Reconnecting,
                                  attempt == 0 ? "connect-attempt" : "reconnect-attempt");
        {
            std::lock_guard<std::mutex> lock(mutex_);
            lastError_.clear();
            responseReady_ = false;
        }

        if (!channel_->open()) {
            connectError = QStringLiteral("Failed to dispatch channel open");
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
            lastError_ = QStringLiteral("Aborted");
            transitionConnectionState(ConnectionState::Failed, "reconnect-aborted");
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    lastError_ = connectError.isEmpty() ? QStringLiteral("Connect timeout") : connectError;
    return false;
}

bool ModbusClient::waitForChannelState(io::ChannelState expectedState,
                                       std::chrono::steady_clock::time_point deadline,
                                       QString* errorOut) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            if (errorOut) {
                *errorOut = QStringLiteral("Aborted");
            }
            return false;
        }
        if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
            return true;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!lastError_.isEmpty()) {
                if (errorOut) {
                    *errorOut = lastError_;
                }
                lastError_.clear();
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            if (errorOut && errorOut->isEmpty()) {
                *errorOut = QStringLiteral("Channel entered error state");
            }
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_until(lock, deadline, [this, expectedState]() {
                return aborted_.load() ||
                    !lastError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error ||
                    channel_->state() == expectedState ||
                    (expectedState == io::ChannelState::Open && channel_->isOpen());
            })) {
            break;
        }
    }

    if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
        return true;
    }
    if (errorOut && errorOut->isEmpty()) {
        *errorOut = QStringLiteral("Connect timeout");
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
            if (!lastError_.isEmpty()) {
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_until(lock, deadline, [this]() {
                return aborted_.load() || writeDrained_ || !lastError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error;
            })) {
            break;
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
    std::unique_lock<std::mutex> lock(mutex_);
    const bool signaled = cv_.wait_until(lock, deadline, [this]() {
            return aborted_.load() || responseReady_ || !lastError_.isEmpty() ||
                channel_->state() == io::ChannelState::Error;
        });
    return signaled || std::chrono::steady_clock::now() < deadline;
}

ModbusResponse ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    aborted_ = false;

    const base::ModbusConfig activeConfig = config_;
    const int retries = std::max(0, activeConfig.retries);
    ModbusResponse lastResponse = ModbusResponse::Error("Unknown error");
    const int requestId = enqueuePendingRequest(request, slaveId);
    transitionTo(RequestState::Idle, "request-start");

    for (int i = 0; i <= retries; ++i) {
        if (aborted_) {
            transitionTo(RequestState::Aborted, "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error("Aborted");
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
                return ModbusResponse::Error("Aborted");
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
            lastError_.clear();
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
        return ModbusResponse::Error(lastError_.isEmpty() ? QStringLiteral("Not connected") : lastError_);
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
            return ModbusResponse::Error("Aborted");
        }
        buffer_.clear();
        responseReady_ = false;
        writeDrained_ = config_.mode != base::ModbusMode::RTU;
        lastError_.clear();
        lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    }

    // 2. 构建 ADU
    int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    if (isRtuBroadcastRequest(targetSlaveId, request.functionCode()) &&
        !isBroadcastWriteFunction(request.functionCode())) {
        transitionTo(RequestState::Failed, "invalid-rtu-broadcast-function");
        return ModbusResponse::Error("RTU broadcast only supports write function codes");
    }
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);
    waitForRtuInterFrameDelay();

    // 3. 发送数据
    if (!channel_->write(adu)) {
        transitionTo(RequestState::Failed, "write-failed");
        return ModbusResponse::Error("Write failed");
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
            const QString error = lastError_.isEmpty()
                ? QStringLiteral("Write drain timeout")
                : lastError_;
            lastError_.clear();
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
        if (!rtuFrameReady && !responseReady_ && lastError_.isEmpty() && !aborted_.load()) {
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(deadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                return ModbusResponse::Error("Timeout");
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
                return ModbusResponse::Error("Timeout");
            }
            continue;
        }
        if (aborted_) {
            spdlog::info("ModbusClient: Aborted during wait");
            transitionTo(RequestState::Aborted, "aborted-during-wait");
            return ModbusResponse::Error("Aborted");
        }
        if (!lastError_.isEmpty()) {
            QString err = lastError_;
            lastError_.clear();
            transitionTo(RequestState::Failed, "channel-error");
            return ModbusResponse::Error(err);
        }

        responseReady_ = false;

        while (true) {
            if (config_.mode == base::ModbusMode::RTU) {
                QByteArray frame;
                if (tryExtractRtuResponseFrameLocked(request, targetSlaveId, frame, droppedInvalidBytes)) {
                    lock.unlock();
                    auto parseResult = transport_->parseResponse(frame);
                    if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                        const auto& pdu = *parseResult.pdu;
                        if (pdu.isException()) {
                            transitionTo(RequestState::Failed, "modbus-exception");
                            return ModbusResponse::Error(QString("Modbus Exception: %1").arg(static_cast<int>(pdu.exceptionCode())));
                        }
                        if (pdu.originalFunctionCode() != request.functionCode()) {
                            lock.lock();
                            continue;
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
                    return ModbusResponse::Error("Response parsing failed");
                }
                if (!buffer_.isEmpty() && isRtuFrameReadyToParseLocked(std::chrono::steady_clock::now())) {
                    buffer_.remove(0, 1);
                    ++droppedInvalidBytes;
                    if (droppedInvalidBytes > app::constants::Constants::Modbus::kMaxDroppedInvalidBytes) {
                        transitionTo(RequestState::Failed, "too-many-invalid-bytes");
                        return ModbusResponse::Error("Too many invalid response bytes");
                    }
                    responseReady_ = !buffer_.isEmpty();
                    continue;
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
                        return ModbusResponse::Error(QString("Modbus Exception: %1").arg(static_cast<int>(pdu.exceptionCode())));
                    }
                    if (pdu.originalFunctionCode() != request.functionCode()) {
                        lock.lock();
                        continue;
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
                    return ModbusResponse::Error("Response parsing failed");
                }
                lock.lock();
                continue;
            } else if (integrity == -1) {
                if (!buffer_.isEmpty()) {
                    buffer_.remove(0, 1);
                    ++droppedInvalidBytes;
                }
                if (droppedInvalidBytes > app::constants::Constants::Modbus::kMaxDroppedInvalidBytes) {
                    transitionTo(RequestState::Failed, "too-many-invalid-bytes");
                    return ModbusResponse::Error("Too many invalid response bytes");
                }
                responseReady_ = !buffer_.isEmpty();
                continue;
            }
            if (config_.mode == base::ModbusMode::RTU && !buffer_.isEmpty() &&
                isRtuFrameReadyToParseLocked(std::chrono::steady_clock::now())) {
                transitionTo(RequestState::Failed, "incomplete-rtu-frame-after-gap");
                return ModbusResponse::Error("Incomplete RTU frame after inter-frame silence");
            }
            break;
        }
        
        if (std::chrono::steady_clock::now() >= deadline) {
            transitionTo(RequestState::Failed, "timeout-full-packet");
            return ModbusResponse::Error("Timeout while waiting for full packet");
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
        (targetSlaveId < app::constants::Constants::Modbus::kMinSlaveId ||
         targetSlaveId > app::constants::Constants::Modbus::kMaxSlaveId)) {
        return QString("Invalid RTU slave id: %1").arg(targetSlaveId);
    }

    const QByteArray data = request.data();
    if (data.size() > app::constants::Constants::Modbus::kMaxPduDataLength) {
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
        if (quantity < app::constants::Constants::Modbus::kMinQuantity) {
            return QString("Read quantity must be at least %1").arg(app::constants::Constants::Modbus::kMinQuantity);
        }
        if (!isAddressRangeValid(startAddress, quantity)) {
            return QString("Read address range exceeds limit: start=%1 quantity=%2")
                .arg(startAddress)
                .arg(quantity);
        }
        if ((request.functionCode() == base::FunctionCode::ReadCoils ||
             request.functionCode() == base::FunctionCode::ReadDiscreteInputs) &&
            quantity > app::constants::Constants::Modbus::kMaxReadBitsQuantity) {
            return QString("Read bit quantity exceeds limit: %1").arg(quantity);
        }
        if ((request.functionCode() == base::FunctionCode::ReadHoldingRegisters ||
             request.functionCode() == base::FunctionCode::ReadInputRegisters) &&
            quantity > app::constants::Constants::Modbus::kMaxReadQuantity) {
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
        if (quantity < app::constants::Constants::Modbus::kMinQuantity) {
            return QString("Write quantity must be at least %1").arg(app::constants::Constants::Modbus::kMinQuantity);
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
            if (quantity > app::constants::Constants::Modbus::kMaxWriteCoilsQuantity) {
                return QString("Write coil quantity exceeds limit: %1").arg(quantity);
            }
            const int expectedByteCount = (static_cast<int>(quantity) + 7) / 8;
            if (declaredByteCount != expectedByteCount) {
                return QString("Write coil byte count mismatch: declared %1, expected %2")
                    .arg(declaredByteCount)
                    .arg(expectedByteCount);
            }
        } else {
            if (quantity > app::constants::Constants::Modbus::kMaxWriteRegistersQuantity) {
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
    if (tcpMbapLength > app::constants::Constants::Modbus::kMaxTcpMbapLength) {
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
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_until(lock, deadline, [this]() {
        return aborted_.load();
    });
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
    if (config_.mode != base::ModbusMode::RTU || buffer_.isEmpty()) {
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

std::optional<int> ModbusClient::expectedRtuResponseLength(const base::Pdu& request) const {
    if (config_.mode != base::ModbusMode::RTU) {
        return std::nullopt;
    }

    uint16_t quantity = 0;
    switch (request.functionCode()) {
    case base::FunctionCode::ReadCoils:
    case base::FunctionCode::ReadDiscreteInputs:
        if (!readBigEndianUInt16(request.data(), 2, quantity)) {
            return std::nullopt;
        }
        return 1 + 1 + 1 + ((static_cast<int>(quantity) + 7) / 8) + 2;
    case base::FunctionCode::ReadHoldingRegisters:
    case base::FunctionCode::ReadInputRegisters:
        if (!readBigEndianUInt16(request.data(), 2, quantity)) {
            return std::nullopt;
        }
        return 1 + 1 + 1 + (static_cast<int>(quantity) * 2) + 2;
    case base::FunctionCode::WriteSingleCoil:
    case base::FunctionCode::WriteSingleRegister:
    case base::FunctionCode::WriteMultipleCoils:
    case base::FunctionCode::WriteMultipleRegisters:
        return 8;
    default:
        return std::nullopt;
    }
}

bool ModbusClient::tryExtractRtuResponseFrameLocked(const base::Pdu& request,
                                                    int targetSlaveId,
                                                    QByteArray& frame,
                                                    int& droppedInvalidBytes) {
    if (config_.mode != base::ModbusMode::RTU || buffer_.size() < 5) {
        return false;
    }

    const auto expectedLength = expectedRtuResponseLength(request);
    const uint8_t expectedSlaveId = static_cast<uint8_t>(targetSlaveId);
    const uint8_t expectedFunction = static_cast<uint8_t>(request.functionCode());

    auto candidateMatches = [this,
                             expectedSlaveId,
                             expectedFunction](const QByteArray& candidate, bool exceptionFrame) {
        if (candidate.size() < 5 || static_cast<uint8_t>(candidate[0]) != expectedSlaveId) {
            return false;
        }
        const uint8_t actualFunction = static_cast<uint8_t>(candidate[1]);
        const uint8_t wantedFunction = exceptionFrame
            ? static_cast<uint8_t>(expectedFunction | 0x80)
            : expectedFunction;
        return actualFunction == wantedFunction && transport_->checkIntegrity(candidate) == candidate.size();
    };

    for (int offset = 0; offset <= buffer_.size() - 5; ++offset) {
        const int remaining = buffer_.size() - offset;
        if (remaining < 5) {
            break;
        }

        const QByteArray exceptionCandidate = buffer_.mid(offset, 5);
        if (candidateMatches(exceptionCandidate, true)) {
            if (offset > 0) {
                droppedInvalidBytes += offset;
            }
            buffer_.remove(0, offset + exceptionCandidate.size());
            frame = exceptionCandidate;
            return true;
        }

        if (expectedLength && remaining >= *expectedLength) {
            const QByteArray normalCandidate = buffer_.mid(offset, *expectedLength);
            if (candidateMatches(normalCandidate, false)) {
                if (offset > 0) {
                    droppedInvalidBytes += offset;
                }
                buffer_.remove(0, offset + normalCandidate.size());
                frame = normalCandidate;
                return true;
            }
        }
    }

    return false;
}

void ModbusClient::onDataReceived(QByteArrayView data) {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::info("ModbusClient: Data received, size={}, notifying loop", data.size());
    buffer_.append(data);
    if (config_.mode == base::ModbusMode::RTU) {
        lastRtuByteReceivedAt_ = std::chrono::steady_clock::now();
    }
    responseReady_ = true; // 即使不完整也唤醒，由工作线程检查完整性
    cv_.notify_one();
}

void ModbusClient::onError(const QString& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_ = error;
    cv_.notify_one();
}

} // namespace modbus::session
