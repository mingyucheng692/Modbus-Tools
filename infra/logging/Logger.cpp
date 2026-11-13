// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "Logger.h"

#include "core/Config.h"
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QFile>
#include <QCoreApplication>
#include <QtGlobal>
#include <cstdio>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {

constexpr auto kDefaultLogLevel = spdlog::level::info;

// Controlled by CMake option MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS.
// ON:  flush after every log call (verbose diagnostics mode).
// OFF: flush on warn or above (production default).
// Call sites use SPDLOG_DEBUG() which is compiled out by SPDLOG_ACTIVE_LEVEL
// when the macro is below SPDLOG_LEVEL_DEBUG.
#if defined(MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS) && MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS
constexpr auto kDefaultFlushLevel = spdlog::level::info;
#else
constexpr auto kDefaultFlushLevel = spdlog::level::warn;
#endif

[[nodiscard]] static spdlog::filename_t encodePathForSpdlog(const QString& path)
{
#if defined(SPDLOG_WCHAR_FILENAMES)
    return QDir::toNativeSeparators(path).toStdWString();
#else
    return QDir::toNativeSeparators(path).toStdString();
#endif
}

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
        // Double-write to stderr: the async queue may drop or not yet have
        // flushed this message before abort(), and stderr survives in crash
        // dumps / console output. Flush before abort so the queued tail is
        // persisted — the dying moments carry the highest diagnostic value.
        std::fprintf(stderr, "Qt FATAL [%s:%d:%s] %s\n", file, line, category, text.c_str());
        logger->critical("Qt [{}:{}:{}] {}", file, line, category, text);
        logger->flush();
        abort();
    }
}

bool ensureLogDirectoryExists(const QString& logDir, QString* errorMessage)
{
    if (logDir.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QCoreApplication::translate("logging", "Log directory path is empty.");
        }
        return false;
    }

    QDir dir(logDir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (errorMessage != nullptr) {
            *errorMessage = QCoreApplication::translate("logging", "Failed to create log directory: %1").arg(logDir);
        }
        return false;
    }
    return true;
}

QString buildTargetLogPath(const QDir& dir)
{
    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss'Z'");
    const QString fileName = QStringLiteral("modbus-tools_%1_%2.log")
                                 .arg(timestamp)
                                 .arg(QCoreApplication::applicationPid());
    return dir.filePath(fileName);
}

bool probeTargetLogWritable(const QString& targetLogPath, QString* errorMessage)
{
    // Verify the exact target file can be opened before constructing rotating_file_sink_mt,
    // because the repo builds with -fno-exceptions and sink creation failures would std::abort().
    const bool existedBeforeProbe = QFile::exists(targetLogPath);
    QFile probeFile(targetLogPath);
    if (!probeFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        if (errorMessage != nullptr) {
            *errorMessage = QCoreApplication::translate("logging", "Log file is not writable: %1").arg(targetLogPath);
        }
        return false;
    }
    probeFile.close();
    if (!existedBeforeProbe) {
        QFile::remove(targetLogPath);
    }
    return true;
}

void pruneOldLogs(QDir& dir, int maxAllowedFiles)
{
    dir.setNameFilters(QStringList() << QStringLiteral("modbus-tools_*.log*"));
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed); // Oldest files first

    const QFileInfoList fileList = dir.entryInfoList();
    if (fileList.size() >= maxAllowedFiles) {
        const int filesToDelete = fileList.size() - maxAllowedFiles + 1;
        for (int i = 0; i < filesToDelete; ++i) {
            QFile::remove(fileList[i].absoluteFilePath());
        }
    }
}

spdlog::level::level_enum resolveEffectiveLogLevel()
{
    auto effectiveLogLevel = kDefaultLogLevel;
    const QByteArray envLevel = qgetenv("MODBUS_TOOLS_LOG_LEVEL").toUpper();
    if (envLevel.isEmpty()) {
        return effectiveLogLevel;
    }
    if (envLevel == "TRACE") return spdlog::level::trace;
    if (envLevel == "DEBUG") return spdlog::level::debug;
    if (envLevel == "INFO") return spdlog::level::info;
    if (envLevel == "WARN" || envLevel == "WARNING") return spdlog::level::warn;
    if (envLevel == "ERR" || envLevel == "ERROR") return spdlog::level::err;
    if (envLevel == "CRITICAL" || envLevel == "FATAL") return spdlog::level::critical;
    if (envLevel == "OFF") return spdlog::level::off;
    return effectiveLogLevel;
}

bool Init(const QString& logDir, QString* errorMessage) noexcept
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (!ensureLogDirectoryExists(logDir, errorMessage)) {
        return false;
    }

    QDir dir(logDir);
    const QString targetLogPath = buildTargetLogPath(dir);
    if (!probeTargetLogWritable(targetLogPath, errorMessage)) {
        return false;
    }

    // Global log disk-space protection: prune oldest historical logs before creating current sink.
    const int maxAllowedFiles = static_cast<int>(config::Logging::kMaxRotatedFiles);
    pruneOldLogs(dir, maxAllowedFiles);

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    const spdlog::filename_t filePath = encodePathForSpdlog(targetLogPath);

    // rotating_file_sink bounds per-file size during long-running sessions.
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

    // Overflow policy = overrun_oldest: when the queue is full, drop the
    // oldest message instead of blocking the caller. The Modbus worker
    // thread logs on every request; with block policy a stalled file sink
    // (antivirus scan, cloud-sync contention) would stall the worker and
    // corrupt the very RTT measurements this tool exists to take. An
    // observability system must never interfere with the observed link —
    // losing the oldest log line is always preferable to blocking the
    // measured path.
    auto logger = std::make_shared<spdlog::async_logger>(
        "default",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::overrun_oldest);
    logger->set_formatter(std::make_unique<spdlog::pattern_formatter>(
        "%Y-%m-%d %H:%M:%S.%eZ [%t] [%^%l%$] %v",
        spdlog::pattern_time_type::utc));

    const auto effectiveLogLevel = resolveEffectiveLogLevel();
    logger->set_level(effectiveLogLevel);
    logger->flush_on(kDefaultFlushLevel);
    spdlog::set_error_handler([](const std::string& message) {
        fprintf(stderr, "spdlog failure: %s\n", message.c_str());
    });

    spdlog::set_default_logger(logger);
    spdlog::set_level(effectiveLogLevel);

    qInstallMessageHandler(QtMessageHandler);
    return true;
}

void Shutdown() noexcept
{
    qInstallMessageHandler(nullptr);
    if (auto logger = spdlog::default_logger()) {
        logger->flush();
    }
    spdlog::shutdown();
}

void SetLogLevel(spdlog::level::level_enum level) noexcept {
    spdlog::set_level(level);
    if (auto logger = spdlog::default_logger()) {
        logger->set_level(level);
    }
}

[[nodiscard]] spdlog::level::level_enum GetLogLevel() noexcept {
    if (auto logger = spdlog::default_logger()) {
        return logger->level();
    }
    return spdlog::get_level();
}

}
