#pragma once

#ifdef RTW_USE_FIXED_POINT
#include "fixed_point/fixed_point.h"
#endif

#include <limits>

namespace rtw::sw_renderer
{

#ifdef RTW_USE_FIXED_POINT
using single_precision = fixed_point::FixedPoint16;
using double_precision = fixed_point::FixedPoint32;

// unit of least precision
constexpr single_precision ULP{1};// NOLINT(clang-diagnostic-unused-const-variable)
#else
using single_precision = float;
using double_precision = double;

// unit of least precision
constexpr float ULP{std::numeric_limits<float>::epsilon()}; // NOLINT(clang-diagnostic-unused-const-variable)
#endif

} // namespace rtw::sw_renderer
