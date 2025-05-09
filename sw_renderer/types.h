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

using Vector2F = math::Vector2<single_precision>;
using Vector2D = math::Vector2<double_precision>;

using Vector3F = math::Vector3<single_precision>;
using Vector3D = math::Vector3<double_precision>;

using Vector4F = math::Vector4<single_precision>;
using Vector4D = math::Vector4<double_precision>;

using Point2F = math::Point2<single_precision>;
using Point2D = math::Point2<double_precision>;
using Point2I = math::Point2<std::int32_t>;

using Point3F = math::Point3<single_precision>;
using Point3D = math::Point3<double_precision>;

using Point4F = math::Point4<single_precision>;
using Point4D = math::Point4<double_precision>;

using Matrix3x3F = math::Matrix3x3<single_precision>;
using Matrix3x3D = math::Matrix3x3<double_precision>;

using Matrix4x4F = math::Matrix4x4<single_precision>;
using Matrix4x4D = math::Matrix4x4<double_precision>;

using Frustum3F = math::Frustum3<single_precision>;
using Frustum3D = math::Frustum3<double_precision>;

using AngleF = math::Angle<single_precision>;
using AngleD = math::Angle<double_precision>;

using EulerAnglesF = math::EulerAngles<single_precision>;
using EulerAnglesD = math::EulerAngles<double_precision>;

using BarycentricF = math::Barycentric<single_precision>;
using BarycentricD = math::Barycentric<double_precision>;

using TexCoordF = TexCoord<single_precision>;
using TexCoordD = TexCoord<double_precision>;

namespace angle_literals
{

constexpr AngleF operator"" _degF(long double value) noexcept
{
  return AngleF{math::DEG, static_cast<single_precision>(value)};
}

constexpr AngleD operator"" _degD(unsigned long long value) noexcept
{
  return AngleD{math::DEG, static_cast<double_precision>(value)};
}

constexpr AngleF operator"" _radF(long double value) noexcept
{
  return AngleF{math::RAD, static_cast<single_precision>(value)};
}

constexpr AngleD operator"" _radD(unsigned long long value) noexcept
{
  return AngleD{math::RAD, static_cast<double_precision>(value)};
}

} // namespace angle_literals

} // namespace rtw::sw_renderer
