// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "infra/config/ISettingsService.h"

namespace core::common {

/// Role: Decoupled domain reference to settings service interface.
/// Thread-safety: Thread-compatible interface.
/// Ownership: Observer interface.
using ISettingsService = ::infra::config::ISettingsService;

} // namespace core::common

