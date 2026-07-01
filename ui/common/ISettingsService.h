/**
 * @file ISettingsService.h
 * @brief ui::common::ISettingsService — historical interface kept as a
 *        pure subclass of ::core::common::ISettingsService for source
 *        compatibility with existing forward declarations (`class ISettingsService;`).
 *
 * The canonical interface lives in core/common/ISettingsService.h so that
 * core-layer code can depend on it without a reverse dependency on ui.
 * New code SHOULD depend on ::core::common::ISettingsService directly.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../../core/common/ISettingsService.h"

namespace ui::common {

// Historical name preserved as a subclass so existing forward declarations
// (`class ISettingsService;`) keep compiling. The subclass adds no behavior;
// it exists only to keep the `ui::common::ISettingsService` type name alive.
class ISettingsService : public ::core::common::ISettingsService {};

} // namespace ui::common
