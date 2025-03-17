#pragma once

#include "math/interpolation.h"
#include "math/point.h"

#include <limits>

namespace rtw::math
{

/// A policy is used to indicate whether the calculation should be checked producing a slower but safer result.
/// DONT_CHECK is faster but the result is undefined if the calculation is invalid.
enum class CalculationCheckPolicy : std::uint8_t
{
  CHECK,
  DONT_CHECK
};

/// Returns the intersection factor of the line and the plane.
/// Intersection factor `t` is the factor of the line's direction vector which is calculated as follows:
/// `i = q0 + t * (q1 - q0)`, where `i` is the intersection point.
/// Solving the equation for `t` gives the following result:
/// `t = (p - q0) * n / (q1 - q0) * n`, where `*` is dot product, @c p is a point on the plane, @c n is the plane's
/// The intersesion point can then be calculated by using the linear interpolation function: `i = lerp(q0, q1, t)`.
/// The plane is defined by a point and a normal.
/// The line is defined by two points.
/// @tparam POLICY The policy to check the calculation.
/// If the policy is CalculationCheckPolicy::CHECK, the function returns std::numeric_limits<T>::max if the line is
/// parallel to the plane. If the policy is CalculationCheckPolicy::DONT_CHECK, the function does not check the
/// calculation producing a faster result, but the result is undefined if the line is parallel to the plane.
/// @tparam T The type of the point's coordinates.
/// @tparam N The number of dimensions.
/// @param p A point on the plane.
/// @param n The normal of the plane.
/// @param q0 A point on the line.
/// @param q1 A point on the line.
/// @return The intersection of the line and the plane or std::numeric_limits<T>::max if the line is parallel to the
/// plane.
template <CalculationCheckPolicy POLICY, typename T, std::uint16_t N>
constexpr T intersection_factor(const Point<T, N>& p, const Vector<T, N>& n, const Point<T, N>& q0,
                                const Point<T, N>& q1) noexcept
{
  const auto q0p_dot_n = dot(q0 - p, n);
  const auto q1p_dot_n = dot(q1 - p, n);
  const auto q0q1_dot_n = dot(q0 - q1, n);
  const auto diff = q0p_dot_n - q1p_dot_n;
  if constexpr (POLICY == CalculationCheckPolicy::CHECK)
  {
    if (q0q1_dot_n == T{0})
    {
      return std::numeric_limits<T>::max();
    }
  }
  return q0p_dot_n / diff;
}

/// Returns the intersection point of the line and the plane.
/// The plane is defined by a point and a normal.
/// The line is defined by two points.
/// @tparam POLICY The policy to check the calculation.
/// If the policy is CalculationCheckPolicy::CHECK, the function returns std::numeric_limits<T>::max if the line is
/// parallel to the plane. If the policy is CalculationCheckPolicy::DONT_CHECK, the function does not check the
/// calculation producing a faster result, but the result is undefined if the line is parallel to the plane.
/// @tparam T The type of the point's coordinates.
/// @tparam N The number of dimensions.
/// @param p A point on the plane.
/// @param n The normal of the plane.
/// @param q0 A point on the line.
/// @param q1 A point on the line.
/// @return The intersection point of the line and the plane or std::numeric_limits<T>::max if the line is parallel to
/// the plane.
template <CalculationCheckPolicy POLICY, typename T, std::uint16_t N>
constexpr Point<T, N> intersection(const Point<T, N>& p, const Vector<T, N>& n, const Point<T, N>& q0,
                                   const Point<T, N>& q1) noexcept
{
  const auto t = intersection_factor<POLICY>(p, n, q0, q1);

  if constexpr (POLICY == CalculationCheckPolicy::CHECK)
  {
    if (t == std::numeric_limits<T>::max())
    {
      return Point<T, N>{math::INITIALIZE_WITH_VALUE, std::numeric_limits<T>::max()};
    }
  }

  return lerp(q0, q1, t);
}

/// Returns the intersection factor of the two 2D lines.
/// The lines are defined by two points.
/// Intersection factor `t` is the factor of the line's direction vector which is calculated as follows:
/// `i = p0 + t * (p1 - p0)`, where `i` is the intersection point.
/// Solving the equation for `t` gives the following result:
/// `t = (p0 - q0) x (q0 - q1) / (p0 - p1) x (q0 - q1)`, where `x` is the cross product.
/// The intersesion point can then be calculated by using the linear interpolation function: `i = lerp(p0, p1, t)`.
/// @tparam POLICY The policy to check the calculation.
/// If the policy is CalculationCheckPolicy::CHECK, the function returns std::numeric_limits<T>::max if the lines are
/// parallel. If the policy is CalculationCheckPolicy::DONT_CHECK, the function does not check the calculation producing
/// a faster result, but the result is undefined if the lines are parallel.
/// @tparam T The type of the point's coordinates.
/// @param p0 A point on the first line.
/// @param p1 A point on the first line.
/// @param q0 A point on the second line.
/// @param q1 A point on the second line.
/// @return The intersection factor of the two lines or std::numeric_limits<T>::max if the lines are parallel.
template <CalculationCheckPolicy POLICY, typename T>
constexpr T intersection_factor(const Point2<T>& p0, const Point2<T>& p1, const Point2<T>& q0,
                                const Point2<T>& q1) noexcept
{
  const auto p0p1 = p0 - p1;
  const auto q0q1 = q0 - q1;
  const auto determinant = cross(p0p1, q0q1);
  if constexpr (POLICY == CalculationCheckPolicy::CHECK)
  {
    if (determinant == T{0})
    {
      return std::numeric_limits<T>::max();
    }
  }

  const auto p0q0 = p0 - q0;
  return cross(p0q0, q0q1) / determinant;
}

/// Returns the intersection point of the two 2D lines.
/// The lines are defined by two points.
/// @tparam POLICY The policy to check the calculation.
/// If the policy is CalculationCheckPolicy::CHECK, the function returns std::numeric_limits<T>::max if the lines are
/// parallel. If the policy is CalculationCheckPolicy::DONT_CHECK, the function does not check the calculation producing
/// a faster result, but the result is undefined if the lines are parallel.
/// @tparam T The type of the point's coordinates.
/// @param p0 A point on the first line.
/// @param p1 A point on the first line.
/// @param q0 A point on the second line.
/// @param q1 A point on the second line.
/// @return The intersection point of the two lines or std::numeric_limits<T>::max if the lines are parallel.
template <CalculationCheckPolicy POLICY, typename T>
constexpr Point2<T> intersection(const Point2<T>& p0, const Point2<T>& p1, const Point2<T>& q0,
                                 const Point2<T>& q1) noexcept
{
  const auto t = intersection_factor<POLICY>(p0, p1, q0, q1);

  if constexpr (POLICY == CalculationCheckPolicy::CHECK)
  {
    if (t == std::numeric_limits<T>::max())
    {
      return Point2<T>{math::INITIALIZE_WITH_VALUE, std::numeric_limits<T>::max()};
    }
  }

  return lerp(p0, p1, t);
}

} // namespace rtw::math
