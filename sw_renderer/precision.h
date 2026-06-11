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

/// Unit of least precision (smallest representable increment above zero).
constexpr inline single_precision ULP{1};
#else
using single_precision = float;  ///< Primary scalar type (floating-point mode).
using double_precision = double; ///< Extended scalar type (floating-point mode).

/// Unit of least precision (machine epsilon for float).
constexpr inline float ULP{std::numeric_limits<float>::epsilon()};
#endif

} // namespace rtw::sw_renderer
