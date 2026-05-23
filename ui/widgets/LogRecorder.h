/**
 * @file LogRecorder.h
 * @brief RAII file recorder for persisting monitor log lines to disk.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QFile>
#include <QString>
#include <memory>

class QTextStream;

namespace ui::widgets {

class LogRecorder {
public:
    LogRecorder();
    ~LogRecorder();

    LogRecorder(const LogRecorder&) = delete;
    LogRecorder& operator=(const LogRecorder&) = delete;

    bool start(const QString& filePath);
    void stop();
    bool writeLine(const QString& line);
    bool isRecording() const { return recording_; }
    QString filePath() const { return filePath_; }

private:
    QFile file_;
    std::unique_ptr<QTextStream> stream_;
    bool recording_ = false;
    QString filePath_;
};

} // namespace ui::widgets