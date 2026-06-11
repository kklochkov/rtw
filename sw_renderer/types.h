#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/tex_coord.h"

#include "math/angle.h"
#include "math/barycentric.h"
#include "math/frustum.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"

namespace rtw::sw_renderer
{

// --- Vectors ----------------------------------------------------------------

using Vector2F = math::Vector2<single_precision>; ///< 2D vector (single precision).
using Vector2D = math::Vector2<double_precision>; ///< 2D vector (double precision).

using Vector3F = math::Vector3<single_precision>; ///< 3D vector (single precision).
using Vector3D = math::Vector3<double_precision>; ///< 3D vector (double precision).

using Vector4F = math::Vector4<single_precision>; ///< 4D vector (single precision).
using Vector4D = math::Vector4<double_precision>; ///< 4D vector (double precision).

// --- Points -----------------------------------------------------------------

using Point2F = math::Point2<single_precision>; ///< 2D point (single precision).
using Point2D = math::Point2<double_precision>; ///< 2D point (double precision).
using Point2I = math::Point2<std::int32_t>;     ///< 2D point (integer, for pixel coordinates).

using Point3F = math::Point3<single_precision>; ///< 3D point (single precision).
using Point3D = math::Point3<double_precision>; ///< 3D point (double precision).

using Point4F = math::Point4<single_precision>; ///< 4D homogeneous point (single precision).
using Point4D = math::Point4<double_precision>; ///< 4D homogeneous point (double precision).

// --- Matrices ---------------------------------------------------------------

using Matrix3x3F = math::Matrix3x3<single_precision>; ///< 3x3 matrix (single precision).
using Matrix3x3D = math::Matrix3x3<double_precision>; ///< 3x3 matrix (double precision).

using Matrix4x4F = math::Matrix4x4<single_precision>; ///< 4x4 matrix (single precision).
using Matrix4x4D = math::Matrix4x4<double_precision>; ///< 4x4 matrix (double precision).

// --- Geometry ---------------------------------------------------------------

using Frustum3F = math::Frustum3<single_precision>; ///< 3D view frustum (single precision).
using Frustum3D = math::Frustum3<double_precision>; ///< 3D view frustum (double precision).

// --- Angles -----------------------------------------------------------------

using AngleF = math::Angle<single_precision>; ///< Angle (single precision).
using AngleD = math::Angle<double_precision>; ///< Angle (double precision).

using EulerAnglesF = math::EulerAngles<single_precision>; ///< Euler angles (single precision).
using EulerAnglesD = math::EulerAngles<double_precision>; ///< Euler angles (double precision).

// --- Interpolation ----------------------------------------------------------

using BarycentricF = math::Barycentric<single_precision>; ///< Barycentric coordinates (single precision).
using BarycentricD = math::Barycentric<double_precision>; ///< Barycentric coordinates (double precision).

// --- Texture coordinates ----------------------------------------------------

using TexCoordF = TexCoord<single_precision>; ///< Texture coordinate (single precision).
using TexCoordD = TexCoord<double_precision>; ///< Texture coordinate (double precision).

/// User-defined literals for constructing angles.
namespace angle_literals
{

/// Construct a single-precision angle from degrees (e.g., 90.0_degF).
constexpr AngleF operator"" _degF(long double value) noexcept
{
  return AngleF{math::DEG, static_cast<single_precision>(value)};
}

/// Construct a double-precision angle from degrees (e.g., 90_degD).
constexpr AngleD operator"" _degD(unsigned long long value) noexcept
{
  return AngleD{math::DEG, static_cast<double_precision>(value)};
}

/// Construct a single-precision angle from radians (e.g., 1.57_radF).
constexpr AngleF operator"" _radF(long double value) noexcept
{
  return AngleF{math::RAD, static_cast<single_precision>(value)};
}

/// Construct a double-precision angle from radians (e.g., 1_radD).
constexpr AngleD operator"" _radD(unsigned long long value) noexcept
{
  return AngleD{math::RAD, static_cast<double_precision>(value)};
}

} // namespace angle_literals

} // namespace rtw::sw_renderer
