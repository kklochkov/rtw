#pragma once

#include "math/matrix_operations.h"
#include "math/plane.h"
#include "math/point.h"
#include "math/vector_operations.h"

namespace rtw::math
{

template <typename T, std::uint16_t N>
constexpr T distance_squared(const Point<T, N>& a, const Point<T, N>& b) noexcept
{
  return norm2(a - b);
}

template <typename T, std::uint16_t N>
constexpr T distance(const Point<T, N>& a, const Point<T, N>& b) noexcept
{
  return norm(a - b);
}

template <typename T>
constexpr T signed_distance(const Point3<T>& point, const Plane3<T>& plane) noexcept
{
  return dot(static_cast<const Vector3<T>>(point), plane.normal) + plane.distance;
}

} // namespace rtw::math
