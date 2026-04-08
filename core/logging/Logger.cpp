#include "Logger.h"

#include "AppConstants.h"
#include <QDateTime>
#include <QDir>
#include <QtGlobal>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {

static void QtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    auto logger = spdlog::default_logger();
    if (!logger) {
        return;
    }

    QByteArray utf8 = message.toUtf8();
    std::string text(utf8.constData(), utf8.size());

    switch (type) {
    case QtDebugMsg:
        logger->debug(text);
        break;
    case QtInfoMsg:
        logger->info(text);
        break;
    case QtWarningMsg:
        logger->warn(text);
        break;
    case QtCriticalMsg:
        logger->error(text);
        break;
    case QtFatalMsg:
        logger->critical(text);
        abort();
    }

    Q_UNUSED(context);
}

void Init(const QString& logDir)
{
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 1. 全局防爆盘：扫描历史带有时间戳的日志文件，主动淘汰过旧的文件
    const int maxAllowedFiles = app::constants::Values::Logging::kMaxRotatedFiles;
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
    
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString fileName = QStringLiteral("modbus-tools_%1.log").arg(timestamp);
    auto filePath = dir.filePath(fileName).toStdString();
    
    // rotating_file_sink 负责单次长时运行期间的日志切分防爆盘
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filePath,
        app::constants::Values::Logging::kMaxFileSizeBytes,
        maxAllowedFiles);
    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

    if (!spdlog::thread_pool()) {
        spdlog::init_thread_pool(
            app::constants::Values::Logging::kAsyncQueueSize,
            app::constants::Values::Logging::kAsyncWorkerThreads);
    }

    auto logger = std::make_shared<spdlog::async_logger>(
        "default",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%t] [%n] [%^%l%$] %v");
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);
    spdlog::set_error_handler([](const std::string& message) {
        qWarning("spdlog failure: %s", message.c_str());
    });

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);

    qInstallMessageHandler(QtMessageHandler);
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
