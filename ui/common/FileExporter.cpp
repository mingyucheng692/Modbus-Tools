// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "FileExporter.h"
#include <QFile>
#include <QObject>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>

namespace ui::common {

namespace {

bool writeLinesStream(QIODevice& device, const QStringList& lines, QString* error) {
    QTextStream out(&device);
    out.setEncoding(QStringConverter::Utf8);
    for (const QString& line : lines) {
        out << line << '\n';
    }
    out.flush();
    if (out.status() != QTextStream::Ok) {
        if (error) {
            *error = QObject::tr("Failed while writing text stream");
        }
        return false;
    }
    return true;
}

bool exportAtomicOverwrite(const QString& filePath, const QStringList& lines, QString* error) {
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    if (!writeLinesStream(file, lines, error) || file.error() != QFileDevice::NoError) {
        if (error && error->isEmpty()) {
            *error = file.errorString();
        }
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

bool exportAppend(const QString& filePath, const QStringList& lines, QString* error) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    if (!writeLinesStream(file, lines, error) || !file.flush() || file.error() != QFileDevice::NoError) {
        if (error && error->isEmpty()) {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

} // namespace

bool exportLinesToFile(const QString& filePath,
                       const QStringList& lines,
                       bool append,
                       QString* error) {
    if (error) {
        error->clear();
    }
    if (append) {
        return exportAppend(filePath, lines, error);
    }
    return exportAtomicOverwrite(filePath, lines, error);
}

} // namespace ui::common
