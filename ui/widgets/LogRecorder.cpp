/**
 * @file LogRecorder.cpp
 * @brief Implementation of LogRecorder.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "LogRecorder.h"
#include <QTextStream>
#include <spdlog/spdlog.h>

namespace ui::widgets {

LogRecorder::LogRecorder() = default;

LogRecorder::~LogRecorder() {
    stop();
}

bool LogRecorder::start(const QString& filePath) {
    if (recording_) {
        stop();
    }
    filePath_ = filePath;
    file_.setFileName(filePath);
    if (!file_.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        spdlog::error("LogRecorder: failed to open file {}", filePath.toStdString());
        filePath_.clear();
        return false;
    }
    stream_ = std::make_unique<QTextStream>(&file_);
    recording_ = true;
    return true;
}

void LogRecorder::stop() {
    recording_ = false;
    if (stream_) {
        stream_->flush();
        stream_.reset();
    }
    if (file_.isOpen()) {
        file_.close();
    }
    filePath_.clear();
}

bool LogRecorder::writeLine(const QString& line) {
    if (!recording_ || !stream_) {
        return false;
    }
    *stream_ << line << "\n";
    if (stream_->status() != QTextStream::Ok) {
        spdlog::error("LogRecorder: write failed for file {}", filePath_.toStdString());
        stop();
        return false;
    }
    return true;
}

} // namespace ui::widgets