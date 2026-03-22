#pragma once

#include "math/angle.h"
#include "math/interpolation.h"
#include "math/numeric.h"
#include "math/quaternion.h"
#include "math/vector_operations.h"

namespace rtw::math
{

template <typename T>
constexpr T dot(const Quaternion<T>& lhs, const Quaternion<T>& rhs) noexcept
{
  return dot(lhs.as_matrix(), rhs.as_matrix());
}

template <typename T>
constexpr T norm2(const Quaternion<T>& quaternion) noexcept
{
  return dot(quaternion, quaternion);
}

template <typename T>
constexpr T norm(const Quaternion<T>& quaternion) noexcept
{
  using multiprecision::math::sqrt;
  using std::sqrt;
  return sqrt(norm2(quaternion));
}

template <typename T>
constexpr Quaternion<T> normalize(const Quaternion<T>& quaternion) noexcept
{
  const auto n = norm(quaternion);
  assert(n != T{0});
  return quaternion / n;
}

template <typename T>
constexpr Vector3<T> axis(const Quaternion<T>& quaternion) noexcept
{
  return normalize(quaternion.xyz());
}

template <typename T>
constexpr Quaternion<T> inverse(const Quaternion<T>& quaternion) noexcept
{
  const auto n2 = norm2(quaternion);
  assert(n2 != T{0});
  return quaternion.conjugate() / n2;
}

/// Computes the logarithm of a unit quaternion.
/// The logarithm of a quaternion q = [w, x, y, z] is defined as:
/// log(q) = [0, (theta / n) * v], where n = norm(q), v = [x, y, z], and theta = atan2(n, w).
/// If the quaternion is near zero, the logarithm is defined to be the zero quaternion to avoid numerical instability.
/// @param[in] quaternion The quaternion to compute the logarithm of.
/// @param[in] epsilon The near-zero epsilon to use for the comparison.
/// @return The logarithm of the pure/vector quaternion.
template <typename T>
constexpr Quaternion<T> log(const Quaternion<T>& quaternion, const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  const auto n = norm(quaternion.xyz());
  if (is_near_zero(n, epsilon))
  {
    return Quaternion<T>{ZERO}; // log(identity) = 0
  }

  using multiprecision::math::atan2;
  using std::atan2;

  const auto theta = atan2(n, quaternion.w());
  const auto scale = theta / n;
  return Quaternion<T>{0, quaternion.x() * scale, quaternion.y() * scale, quaternion.z() * scale};
}

/// Computes the exponential of a pure/vector quaternion.
/// The exponential of a quaternion q = [0, x, y, z] is defined as:
/// exp(q) = [cos(theta), (sin(theta) / n) * v], where n = norm(q), v = [x, y, z], and theta = n.
/// If the quaternion is near zero, the exponential is defined to be the identity quaternion to avoid numerical
/// instability.
/// @param[in] quaternion The pure/vector quaternion to compute the exponential of.
/// @param[in] epsilon The near-zero epsilon to use for the comparison.
/// @return The exponential of the pure/vector quaternion.
template <typename T>
constexpr Quaternion<T> exp(const Quaternion<T>& quaternion, const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  const auto n = norm(quaternion.xyz());
  if (is_near_zero(n, epsilon))
  {
    return Quaternion<T>{IDENTITY}; // exp(0) = identity
  }

  using multiprecision::math::cos;
  using multiprecision::math::sin;
  using std::cos;
  using std::sin;

  const auto scale = sin(n) / n;
  return Quaternion<T>{cos(n), quaternion.x() * scale, quaternion.y() * scale, quaternion.z() * scale};
}

template <typename T>
constexpr Quaternion<T> pow(const Quaternion<T>& quaternion, const T t,
                            const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  return exp(log(quaternion, epsilon) * t, epsilon);
}

/// Performs normalized linear interpolation (nlerp) between two quaternions.
/// This is a faster but less accurate alternative to spherical linear interpolation (slerp).
/// The result is not constant speed and does not follow the shortest path on the unit sphere, but it is sufficient for
/// many applications and is more efficient to compute.
/// @param[in] q1 The starting quaternion.
/// @param[in] q2 The ending quaternion.
/// @param[in] t The interpolation parameter, typically in the range [0, 1].
/// @return The interpolated quaternion at parameter t.
template <typename T>
constexpr Quaternion<T> nlerp(const Quaternion<T>& q1, const Quaternion<T>& q2, const T t) noexcept
{
  // Ensure the shortest path.
  if (dot(q1, q2) < T{0})
  {
    return Quaternion<T>{normalize(lerp(q1.as_matrix(), (-q2).as_matrix(), t))};
  }
  return Quaternion<T>{normalize(lerp(q1.as_matrix(), q2.as_matrix(), t))};
}

/// Performs spherical linear interpolation (slerp) between two quaternions.
/// This interpolation method provides constant speed and follows the shortest path on the unit sphere.
/// The slerp is defined as:
/// slerp(q1, q2, t) = (sin((1 - t) * theta) / sin(theta)) * q1 + (sin(t * theta) / sin(theta)) * q2,
/// where theta is the angle between q1 and q2.
/// If the angle is small, slerp can be
/// approximated by nlerp to avoid numerical instability.
/// @note If the quaternions are nearly opposite (cos(theta) < 0), the interpolation will take the long way around the
/// sphere. To fix this, one quaternion is negated to ensure the shortest path is taken.
/// @param[in] q1 The starting quaternion.
/// @param[in] q2 The ending quaternion.
/// @param[in] t The interpolation parameter, typically in the range [0, 1].
/// @param[in] epsilon The near-zero epsilon to avoid numerical instability for small angles.
/// @return The interpolated quaternion at parameter t.
template <typename T>
constexpr Quaternion<T> slerp(const Quaternion<T>& q1, Quaternion<T> q2, const T t,
                              const T epsilon = default_near_zero_epsilon<T>()) noexcept
{
  using multiprecision::math::acos;
  using multiprecision::math::sin;
  using std::acos;
  using std::sin;

  auto cos_theta = dot(q1, q2);

  // If cos_theta is negative, the interpolation will take the long way around the sphere.
  // To fix this, one quaternion must be negated.
  if (cos_theta < T{0})
  {
    q2 = -q2;
    cos_theta = -cos_theta;
  }

  if (cos_theta > (T{1} - epsilon))
  {
    // If the quaternions are very close, use nlerp to avoid numerical instability.
    return nlerp(q1, q2, t);
  }

  // Compute the angle between the quaternions and perform spherical linear interpolation.
  const auto theta = acos(cos_theta);
  const auto sin_theta = sin(theta);
  const auto weight_q1 = sin((T{1} - t) * theta) / sin_theta;
  const auto weight_q2 = sin(t * theta) / sin_theta;

  return weight_q1 * q1 + weight_q2 * q2;
}

} // namespace rtw::math
