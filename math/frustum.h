#pragma once

#include "math/angle.h"
#include "math/matrix.h"
#include "math/plane.h"
#include "math/vector.h"

namespace rtw::math
{

/// A frustum in 3D space.
/// The normals of the planes are pointing towards the positive half-space, i.e. the inside of the frustum.
/// @tparam T The type of the frustum elements.
template <typename T>
struct Frustum3
{
  Plane3<T> left{};
  Plane3<T> right{};
  Plane3<T> top{};
  Plane3<T> bottom{};
  Plane3<T> near{};
  Plane3<T> far{};
};

using Frustum3F = Frustum3<float>;
using Frustum3D = Frustum3<double>;
using Frustum3Q16 = Frustum3<fixed_point::FixedPoint16>;
using Frustum3Q32 = Frustum3<fixed_point::FixedPoint32>;

/// The parameters of a perspective projection.
/// @tparam T The type of the projection parameters.
template <typename T>
struct FrustumParameters
{
  T left{};
  T right{};
  T top{};
  T bottom{};
  T near{};
  T far{};
};

/// Creates the parameters for a perspective projection.
/// @tparam T The type of the projection parameters.
/// @param fov_y The vertical field of view.
/// @param aspect_ratio The aspect ratio of the viewport.
/// @param near The distance to the near plane.
/// @param far The distance to the far plane.
/// @return The parameters for a perspective projection.
template <typename T>
constexpr FrustumParameters<T> make_perspective_parameters(const Angle<T> fov_y, const T aspect_ratio, const T near,
                                                           const T far) noexcept
{
  assert(near > T{0});
  assert(far > near);

  const auto top = near * std::tan(fov_y / T{2});
  const auto left = top * aspect_ratio;
  const auto bottom = -top;
  const auto right = -left;

  return FrustumParameters<T>{left, right, top, bottom, near, far};
}

/// Creates a perspective projection matrix.
/// @tparam T The type of the matrix elements.
/// @param params The parameters of the perspective projection.
/// @return The perspective projection matrix.
template <typename T>
constexpr Matrix4x4<T> make_perspective_projection_matrix(const FrustumParameters<T> params) noexcept
{
  const auto width = std::abs(params.right - params.left);  // assuming that left and right are not symmetric
  const auto height = std::abs(params.top - params.bottom); // assuming that top and bottom are not symmetric
  const auto depth = params.far - params.near;

  assert(width > T{0});
  assert(height > T{0});
  assert(depth > T{0});

  const auto sx = T{2} * params.near / width;
  const auto sy = T{2} * params.near / height;
  const auto sz = -(params.far + params.near) / depth;
  const auto a13 = (params.right + params.left) / width;
  const auto a23 = (params.top + params.bottom) / height;
  const auto tz = -T{2} * params.far * params.near / depth;
  const auto w = T{-1}; // z is flipped in NDC in right-handed coordinate system

  // clang-format off
  return Matrix4x4<T> {
       sx, T{0}, a13, T{0},
     T{0},   sy, a23, T{0},
     T{0}, T{0},  sz,   tz,
     T{0}, T{0},   w, T{0},
  };
  // clang-format on
}

/// Creates a frustum from the parameters of a perspective projection.
/// @tparam T The type of the frustum elements.
/// @param params The parameters of the perspective projection.
/// @return The frustum.
template <typename T>
inline Frustum3<T> make_frustum(const FrustumParameters<T> params) noexcept
{
  assert(params.near > T{0});

  const auto far_near_aspect_ratio = params.far / params.near;
  const auto far_left = params.left * far_near_aspect_ratio;
  const auto far_right = params.right * far_near_aspect_ratio;
  const auto far_top = params.top * far_near_aspect_ratio;
  const auto far_bottom = params.bottom * far_near_aspect_ratio;

  const Vector3<T> top_left{params.left, params.top, params.near};
  const Vector3<T> top_right{params.right, params.top, params.near};
  const Vector3<T> bottom_left{params.left, params.bottom, params.near};
  const Vector3<T> bottom_right{params.right, params.bottom, params.near};
  const Vector3<T> far_top_left{far_left, far_top, params.far};
  const Vector3<T> far_top_right{far_right, far_top, params.far};
  const Vector3<T> far_bottom_left{far_left, far_bottom, params.far};
  const Vector3<T> far_bottom_right{far_right, far_bottom, params.far};

  Frustum3<T> frustum;
  frustum.near.normal = Vector3<T>{T{0}, T{0}, T{-1}};
  frustum.near.distance = params.near;
  frustum.far.normal = Vector3<T>{T{0}, T{0}, T{1}};
  frustum.far.distance = params.far;
  frustum.top.normal = normalize(cross(far_bottom_left - bottom_left, bottom_right - bottom_left));
  frustum.top.distance = 0;
  frustum.bottom.normal = normalize(cross(top_right - top_left, far_top_left - top_left));
  frustum.bottom.distance = 0;
  frustum.left.normal = normalize(cross(far_top_left - top_left, bottom_left - top_left));
  frustum.left.distance = 0;
  frustum.right.normal = normalize(cross(bottom_right - top_right, far_top_right - top_right));
  frustum.right.distance = 0;
  return frustum;
}

/// Extracts the frustum from a projection matrix.
/// The projection matrix must be in column-major order.
/// The exact frustum extraction algorithm is described in the following article:
/// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
/// @tparam T The type of the matrix elements.
/// @param matrix The projection matrix.
/// @return The frustum.
template <typename T>
constexpr Frustum3<T> extract_frustum(const Matrix4x4<T>& matrix) noexcept
{
  const Vector4<T> column0{matrix.column(0)};
  const Vector4<T> column1{matrix.column(1)};
  const Vector4<T> column2{matrix.column(2)};
  const Vector4<T> column3{matrix.column(3)};

  const auto left = column3 + column0;
  const auto right = column3 - column0;
  const auto top = column3 - column1;
  const auto bottom = column3 + column1;
  const auto near = column3 + column2;
  const auto far = column3 - column2;

  Frustum3<T> frustum;
  frustum.left.normal = normalize(left.xyz());
  frustum.left.distance = left.w();
  frustum.right.normal = normalize(right.xyz());
  frustum.right.distance = right.w();
  frustum.top.normal = normalize(top.xyz());
  frustum.top.distance = top.w();
  frustum.bottom.normal = normalize(bottom.xyz());
  frustum.bottom.distance = bottom.w();
  frustum.near.normal = normalize(near.xyz());
  frustum.near.distance = near.w();
  frustum.far.normal = normalize(far.xyz());
  frustum.far.distance = far.w();
  return frustum;
}

} // namespace rtw::math
