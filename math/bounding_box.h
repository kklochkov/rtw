#pragma once

#include "multiprecision/fixed_point.h"

#include <type_traits>

namespace rtw::math
{

template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
struct BoundingBox
{
  T min_x{};
  T min_y{};
  T max_x{};
  T max_y{};
};

using BoundingBoxI = BoundingBox<std::int32_t>;
using BoundingBoxF = BoundingBox<float>;
using BoundingBoxD = BoundingBox<double>;
using BoundingBoxQ16 = BoundingBox<multiprecision::FixedPoint16>;
using BoundingBoxQ32 = BoundingBox<multiprecision::FixedPoint32>;

} // namespace rtw::math
