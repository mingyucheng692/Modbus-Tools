// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QStringList>

namespace ui::common {

/**
 * @brief Exports lines to the target file.
 * @param filePath Destination file path.
 * @param lines Text lines to write.
 * @param append True to append, false to truncate.
 * @param error Optional output pointer for failure description.
 * @return True if succeeded, false otherwise.
 */
[[nodiscard]] bool exportLinesToFile(const QString& filePath,
                                     const QStringList& lines,
                                     bool append = false,
                                     QString* error = nullptr);

} // namespace ui::common
