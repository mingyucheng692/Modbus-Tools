// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QStringList>

namespace ui::common {

// Exports lines to target file path.
[[nodiscard]] bool exportLinesToFile(const QString& filePath,
                                     const QStringList& lines,
                                     bool append = false,
                                     QString* error = nullptr);

} // namespace ui::common
