#pragma once

#ifdef RTW_USE_FIXED_POINT
#include "multiprecision/fixed_point.h"
#endif

#include <limits>

namespace rtw::sw_renderer
{

#ifdef RTW_USE_FIXED_POINT
using single_precision = multiprecision::FixedPoint16;
using double_precision = multiprecision::FixedPoint32;

// unit of least precision
constexpr inline single_precision ULP{1};
#else
using single_precision = float;
using double_precision = double;

// unit of least precision
constexpr inline float ULP{std::numeric_limits<float>::epsilon()};
#endif

} // namespace rtw::sw_renderer
