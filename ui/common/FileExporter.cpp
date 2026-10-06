/**
 * @file FileExporter.cpp
 * @brief Implementation of FileExporter utility.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FileExporter.h"
#include <QFile>
#include <QTextStream>

namespace ui::common {

bool exportLinesToFile(const QString& filePath,
                       const QStringList& lines,
                       bool append,
                       QString* error) {
    QFile file(filePath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    mode |= append ? QIODevice::Append : QIODevice::Truncate;

    if (!file.open(mode)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }

    QTextStream out(&file);
    for (const QString& line : lines) {
        out << line << '\n';
    }

    return true;
}

} // namespace ui::common
