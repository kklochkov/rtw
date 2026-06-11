#pragma once

#ifdef RTW_USE_FIXED_POINT
#include "multiprecision/fixed_point.h"
#endif

#include <limits>

namespace rtw::sw_renderer
{

#ifdef RTW_USE_FIXED_POINT
using single_precision = multiprecision::FixedPoint16; ///< Primary scalar type (fixed-point mode).
using double_precision = multiprecision::FixedPoint32; ///< Extended scalar type (fixed-point mode).
#else
using single_precision = float;  ///< Primary scalar type (floating-point mode).
using double_precision = double; ///< Extended scalar type (floating-point mode).
#endif

/// Unit of least precision (machine epsilon for float).
constexpr inline single_precision ULP{std::numeric_limits<single_precision>::epsilon()};

} // namespace rtw::sw_renderer
