#pragma once

#include "constants/time_constants.h"

namespace rtw::render_constants
{

/// Target display refresh rate in frames per second.
constexpr inline float TARGET_FRAME_RATE{60.0F};

/// Target time budget per frame, derived from TARGET_FRAME_RATE.
constexpr inline time_constants::Milliseconds TARGET_FRAME_TIME{1000.0F / TARGET_FRAME_RATE};

/// Conversion factor: number of screen pixels per world-space meter.
constexpr inline float PIXELS_PER_METER{100.0F};

static_assert(TARGET_FRAME_RATE > 0.0F, "TARGET_FRAME_RATE must be positive");
static_assert(TARGET_FRAME_TIME.count() > 0.0F, "TARGET_FRAME_TIME must be positive");
static_assert(PIXELS_PER_METER > 0.0F, "PIXELS_PER_METER must be positive");

} // namespace rtw::render_constants
