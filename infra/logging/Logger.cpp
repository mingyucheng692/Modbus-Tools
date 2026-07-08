/**
 * @file Logger.cpp
 * @brief Implementation of Logger.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "Logger.h"

#include "core/Config.h"
#include "infra/platform/PlatformEncoding.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QtGlobal>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {

constexpr auto kDefaultLogLevel = spdlog::level::info;

// Controlled by CMake option MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS.
// ON:  flush after every log call (verbose diagnostics mode).
// OFF: flush on warn or above (production default — see docs/logging-strategy.md).
#if defined(MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS) && MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS
constexpr auto kDefaultFlushLevel = spdlog::level::info;
#else
constexpr auto kDefaultFlushLevel = spdlog::level::warn;
#endif

static void QtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    auto logger = spdlog::default_logger();
    if (!logger) {
        return;
    }

    QByteArray utf8 = message.toUtf8();
    std::string text(utf8.constData(), utf8.size());

    const char* file = context.file ? context.file : "?";
    const int line = context.line;
    const char* category = context.category ? context.category : "default";

    switch (type) {
    case QtDebugMsg:
        logger->debug("Qt [{}:{}:{}] {}", file, line, category, text);
        break;
    case QtInfoMsg:
        logger->info("Qt [{}:{}:{}] {}", file, line, category, text);
        break;
    case QtWarningMsg:
        logger->warn("Qt [{}:{}:{}] {}", file, line, category, text);
        break;
    case QtCriticalMsg:
        logger->error("Qt [{}:{}:{}] {}", file, line, category, text);
        break;
    case QtFatalMsg:
        logger->critical("Qt [{}:{}:{}] {}", file, line, category, text);
        abort();
    }
}

bool ensureLogDirectoryWritable(const QString& logDir, QString* errorMessage)
{
    if (logDir.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QCoreApplication::translate("logging", "Log directory path is empty.");
        }
        return false;
    }

    QDir dir(logDir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (errorMessage) {
            *errorMessage = QCoreApplication::translate("logging", "Failed to create log directory: %1").arg(logDir);
        }
        return false;
    }

    const QString probePath = dir.filePath(QStringLiteral(".write_probe"));
    QFile probeFile(probePath);
    if (!probeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QCoreApplication::translate("logging", "Log directory is not writable: %1").arg(logDir);
        }
        return false;
    }
    probeFile.close();
    probeFile.remove();
    return true;
}

bool Init(const QString& logDir, QString* errorMessage) noexcept
{
    if (errorMessage) {
        errorMessage->clear();
    }

    if (!ensureLogDirectoryWritable(logDir, errorMessage)) {
        return false;
    }

    QDir dir(logDir);

    // 1. 全局防爆盘：扫描历史带有时间戳的日志文件，主动淘汰过旧的文件
    const int maxAllowedFiles = config::Logging::kMaxRotatedFiles;
    dir.setNameFilters(QStringList() << QStringLiteral("modbus-tools_*.log*"));
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed); // 最旧的文件在前面

    QFileInfoList fileList = dir.entryInfoList();
    if (fileList.size() >= maxAllowedFiles) {
        int filesToDelete = fileList.size() - maxAllowedFiles + 1; // 多删一个为新日志腾出空间
        for (int i = 0; i < filesToDelete; ++i) {
            QFile::remove(fileList[i].absoluteFilePath());
        }
    }

    // 2. 初始化本次运行的日志文件（带时间戳）
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss'Z'");
    const QString fileName = QStringLiteral("modbus-tools_%1.log").arg(timestamp);
    const spdlog::filename_t filePath = infra::platform::encodePathForSpdlog(dir.filePath(fileName));
    
    // rotating_file_sink 负责单次长时运行期间的日志切分防爆盘
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filePath,
        config::Logging::kMaxFileSizeBytes,
        maxAllowedFiles);
    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(
            config::Logging::kAsyncQueueSize,
            config::Logging::kAsyncWorkerThreads);
    }

    auto logger = std::make_shared<spdlog::async_logger>(
        "default",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_formatter(std::make_unique<spdlog::pattern_formatter>(
        "%Y-%m-%d %H:%M:%S.%eZ [%t] [%^%l%$] %v",
        spdlog::pattern_time_type::utc));
    logger->set_level(kDefaultLogLevel);
    logger->flush_on(kDefaultFlushLevel);
    spdlog::set_error_handler([](const std::string& message) {
        fprintf(stderr, "spdlog failure: %s\n", message.c_str());
    });

    spdlog::set_default_logger(logger);
    spdlog::set_level(kDefaultLogLevel);

    qInstallMessageHandler(QtMessageHandler);
    return true;
}

void Shutdown()
{
    spdlog::shutdown();
}

void Flush()
{
    spdlog::default_logger()->flush();
}

void SetLevel(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
    auto logger = spdlog::default_logger();
    if (logger) {
        logger->set_level(level);
    }
}

std::shared_ptr<spdlog::logger> Get()
{
    return spdlog::default_logger();
}

}
