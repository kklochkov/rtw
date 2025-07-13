#pragma once

#include "constants/time_constants.h"

namespace rtw::render_constants
{

constexpr float TARGET_FRAME_RATE{60.0F};
constexpr time_constants::Milliseconds TARGET_FRAME_TIME{1000.0F / TARGET_FRAME_RATE};

constexpr float PIXELS_PER_METER{100.0F};

} // namespace rtw::render_constants
