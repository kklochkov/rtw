#pragma once

#include "math/angle.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"

namespace rtw::sw_renderer
{

/// A plane in Hessian normal form.
/// The normal vector is pointing towards the positive half-space, i.e. the inside of the plane.
/// The distance is the distance from the origin to the plane and is positive.
/// @tparam T The type of the plane elements.
template <typename T>
struct Plane3
{
  math::Vector3<T> normal{};
  T distance{};
};

using Plane3f = Plane3<float>;
using Plane3d = Plane3<double>;

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

using Frustum3f = Frustum3<float>;
using Frustum3d = Frustum3<double>;

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
constexpr FrustumParameters<T> make_perspective_parameters(const math::Angle<T> fov_y, const T aspect_ratio, const T near,
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
constexpr math::Matrix4x4<T> make_perspective_projection_matrix(const FrustumParameters<T> params) noexcept
{
  const auto width = std::abs(params.right - params.left);  // assuming that left and right are not symmetric
  const auto height = std::abs(params.top - params.bottom); // assuming that top and bottom are not symmetric
  const auto depth = params.far - params.near;
  const auto sx = T{2} * params.near / width;
  const auto sy = T{2} * params.near / height;
  const auto sz = -(params.far + params.near) / depth;
  const auto a13 = (params.right + params.left) / width;
  const auto a23 = (params.top + params.bottom) / height;
  const auto tz = -T{2} * params.far * params.near / depth;
  const auto w = T{-1}; // z is flipped in NDC in right-handed coordinate system

  // clang-format off
  return math::Matrix4x4<T> {
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
  const auto far_near_aspect_ratio = params.far / params.near;
  const auto far_left = params.left * far_near_aspect_ratio;
  const auto far_right = params.right * far_near_aspect_ratio;
  const auto far_top = params.top * far_near_aspect_ratio;
  const auto far_bottom = params.bottom * far_near_aspect_ratio;

  const math::Vector3<T> top_left{params.left, params.top, params.near};
  const math::Vector3<T> top_right{params.right, params.top, params.near};
  const math::Vector3<T> bottom_left{params.left, params.bottom, params.near};
  const math::Vector3<T> bottom_right{params.right, params.bottom, params.near};
  const math::Vector3<T> far_top_left{far_left, far_top, params.far};
  const math::Vector3<T> far_top_right{far_right, far_top, params.far};
  const math::Vector3<T> far_bottom_left{far_left, far_bottom, params.far};
  const math::Vector3<T> far_bottom_right{far_right, far_bottom, params.far};

  Frustum3<T> frustum;
  frustum.near.normal = math::Vector3<T>{T{0}, T{0}, T{-1}};
  frustum.near.distance = params.near;
  frustum.far.normal = math::Vector3<T>{T{0}, T{0}, T{1}};
  frustum.far.distance = params.far;
  frustum.top.normal = math::normalize(math::cross(far_bottom_left - bottom_left, bottom_right - bottom_left));
  frustum.top.distance = 0;
  frustum.bottom.normal = math::normalize(math::cross(top_right - top_left, far_top_left - top_left));
  frustum.bottom.distance = 0;
  frustum.left.normal = math::normalize(math::cross(far_top_left - top_left, bottom_left - top_left));
  frustum.left.distance = 0;
  frustum.right.normal = math::normalize(math::cross(bottom_right - top_right, far_top_right - top_right));
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
constexpr Frustum3<T> extract_frustum(const math::Matrix4x4<T>& matrix) noexcept
{
  const auto column0 = math::Vector4<T>{matrix.column(0)};
  const auto column1 = math::Vector4<T>{matrix.column(1)};
  const auto column2 = math::Vector4<T>{matrix.column(2)};
  const auto column3 = math::Vector4<T>{matrix.column(3)};

  const auto left = column3 + column0;
  const auto right = column3 - column0;
  const auto top = column3 - column1;
  const auto bottom = column3 + column1;
  const auto near = column3 + column2;
  const auto far = column3 - column2;

  Frustum3<T> frustum;
  frustum.left.normal = math::normalize(left.xyz());
  frustum.left.distance = left.w();
  frustum.right.normal = math::normalize(right.xyz());
  frustum.right.distance = right.w();
  frustum.top.normal = math::normalize(top.xyz());
  frustum.top.distance = top.w();
  frustum.bottom.normal = math::normalize(bottom.xyz());
  frustum.bottom.distance = bottom.w();
  frustum.near.normal = math::normalize(near.xyz());
  frustum.near.distance = near.w();
  frustum.far.normal = math::normalize(far.xyz());
  frustum.far.distance = far.w();
  return frustum;
}

/// Creates a matrix that transforms from world space to view space.
/// @tparam T The type of the matrix elements.
/// @param eye The position of the camera.
/// @param target The position the camera is looking at.
/// @param up The up vector of the camera.
/// @return The view matrix.
template <typename T>
constexpr math::Matrix4x4<T> make_screen_space_matrix(const std::size_t width, const std::size_t height) noexcept
{
  const auto tx = (width - 1) / T{2};
  const auto ty = (height - 1) / T{2};
  const auto sx = tx;
  const auto sy = -ty; // y axis is flipped in screen space
  // clang-format off
  return math::Matrix4x4<T>{
        sx, T{0}, T{0},   tx,
      T{0},   sy, T{0},   ty,
      T{0}, T{0}, T{1}, T{0},
      T{0}, T{0}, T{0}, T{1},
  };
  // clang-format on
}

template <typename T>
constexpr math::Point4<T> ndc_to_screen_space(const math::Point4<T>& point,
                                              const math::Matrix4x4<T>& screen_space_matrix) noexcept
{
  const auto w = point.w(); // original w is needed for depth buffer and perspective correct interpolation
  const auto result = screen_space_matrix * point;
  return math::Point4<T>{result.x(), result.y(), result.z(), w};
}

} // namespace rtw::sw_renderer
