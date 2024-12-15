#pragma once

#include "math/angle.h"
#include "math/point.h"

namespace rtw::sw_renderer
{

/// Make a look-at matrix.
/// @tparam T The type of the elements.
/// @param[in] eye The position of the camera.
/// @param[in] target The position of the target at which the camera is looking.
/// @param[in] up The up vector that defines the orientation of the camera.
/// @return The look-at matrix.
template <typename T>
constexpr math::Matrix4x4<T> make_look_at(const math::Point3<T>& eye, const math::Point3<T>& target,
                                          const math::Vector3<T>& up = math::Vector3<T>{T{0}, T{1}, T{0}}) noexcept
{
  const auto z = math::normalize(eye - target);
  const auto x = math::normalize(math::cross(up, z));
  const auto y = math::cross(z, x);
  const auto e = static_cast<math::Vector3<T>>(eye);
  const auto t = math::Vector3<T>{-math::dot(x, e), -math::dot(y, e), -math::dot(z, e)};
  // clang-format off
  return math::Matrix4x4<T>{
    x.x(), x.y(), x.z(), t.x(),
    y.x(), y.y(), y.z(), t.y(),
    z.x(), z.y(), z.z(), t.z(),
     T{0},  T{0},  T{0},  T{1},
  };
  // clang-format on
}

struct Camera
{
  math::Point3f position{};
  math::Vector3f direction{};
  math::Vector3f velocity{};
  math::EulerAnglesf rotation{};
};

} // namespace rtw::sw_renderer
