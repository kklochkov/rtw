#pragma once

#include "math/angle.h"
#include "math/matrix.h"
#include "math/matrix_operations.h"
#include "math/vector.h"
#include "math/vector_operations.h"
#include "multiprecision/math.h"

namespace rtw::math::transform3
{

/// Make a homogeneous 3D matrix from a 3D matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The 3D matrix.
/// @return The homogeneous 3D matrix.
template <typename T>
constexpr Matrix4x4<T> make_homogeneous(const Matrix3x3<T>& matrix) noexcept
{
  // clang-format off
  return Matrix4x4<T>{
    matrix(0, 0), matrix(0, 1), matrix(0, 2), T{0},
    matrix(1, 0), matrix(1, 1), matrix(1, 2), T{0},
    matrix(2, 0), matrix(2, 1), matrix(2, 2), T{0},
            T{0},         T{0},         T{0}, T{1},
  };
  // clang-format on
}

/// A 3D scaling matrix.
/// @tparam T The type of the elements.
/// @param[in] sx The scaling factor in the x direction.
/// @param[in] sy The scaling factor in the y direction.
/// @param[in] sz The scaling factor in the z direction.
/// @return The 3D scaling matrix.
template <typename T>
constexpr Matrix3x3<T> make_scale(const T sx, const T sy, const T sz) noexcept
{
  // clang-format off
  return Matrix3x3<T>{
       sx, T{0}, T{0},
     T{0},   sy, T{0},
     T{0}, T{0},   sz,
  };
  // clang-format on
}

/// A 3D scaling matrix.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling vector.
/// @return The 3D scaling matrix.
template <typename T>
constexpr Matrix3x3<T> make_scale(const Vector3<T>& scale) noexcept
{
  return make_scale(scale.x(), scale.y(), scale.z());
}

/// A 3D rotation matrix around the x-axis in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_rotation_x(const Angle<T> roll) noexcept
{
  using rtw::multiprecision::math::cos;
  using rtw::multiprecision::math::sin;
  using std::cos;
  using std::sin;

  const auto c = cos(static_cast<T>(roll));
  const auto s = sin(static_cast<T>(roll));
  // clang-format off
  return Matrix3x3<T>{
    T{1}, T{0}, T{0},
    T{0},    c,   -s,
    T{0},    s,    c,
  };
  // clang-format on
}

/// An SO(3) rotation around the x-axis in right-handed 3D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_so3_x(const Angle<T> roll) noexcept
{
  return make_rotation_x(roll);
}

/// A 3D rotation matrix around the y-axis in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] pitch The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_rotation_y(const Angle<T> pitch) noexcept
{
  using rtw::multiprecision::math::cos;
  using rtw::multiprecision::math::sin;
  using std::cos;
  using std::sin;

  const auto c = cos(static_cast<T>(pitch));
  const auto s = sin(static_cast<T>(pitch));
  // clang-format off
  return Matrix3x3<T>{
     c, T{0},    s,
  T{0}, T{1}, T{0},
    -s, T{0},    c,
  };
  // clang-format on
}

/// An SO(3) rotation around the y-axis in right-handed 3D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] pitch The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_so3_y(const Angle<T> pitch) noexcept
{
  return make_rotation_y(pitch);
}

/// A 3D rotation matrix around the z-axis in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] yaw The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_rotation_z(const Angle<T> yaw) noexcept
{
  using rtw::multiprecision::math::cos;
  using rtw::multiprecision::math::sin;
  using std::cos;
  using std::sin;

  const auto c = cos(static_cast<T>(yaw));
  const auto s = sin(static_cast<T>(yaw));
  // clang-format off
  return Matrix3x3<T>{
         c,   -s, T{0},
         s,    c, T{0},
      T{0}, T{0}, T{1},
  };
  // clang-format on
}

/// An SO(3) rotation around the z-axis in right-handed 3D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] yaw The angle of the rotation.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_so3_z(const Angle<T> yaw) noexcept
{
  return make_rotation_z(yaw);
}

/// A 3D rotation matrix around given axes in right-handed coordinate system.
/// The order of the rotations is determined by the rotation convention.
/// @tparam T The type of the elements.
/// @tparam CONVENTION The rotation convention to use for the order of the rotations.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @return The 3D rotation matrix.
template <typename T, RotationConvention CONVENTION>
constexpr Matrix3x3<T> make_rotation(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw) noexcept
{
  if constexpr ((CONVENTION == RotationConvention::XYZ) || (CONVENTION == RotationConvention::ROLL_PITCH_YAW))
  {
    return make_rotation_z(yaw) * make_rotation_y(pitch) * make_rotation_x(roll);
  }

  if constexpr ((CONVENTION == RotationConvention::YZX) || (CONVENTION == RotationConvention::PITCH_YAW_ROLL))
  {
    return make_rotation_x(roll) * make_rotation_z(yaw) * make_rotation_y(pitch);
  }

  if constexpr ((CONVENTION == RotationConvention::ZXY) || (CONVENTION == RotationConvention::YAW_ROLL_PITCH))
  {
    return make_rotation_y(pitch) * make_rotation_x(roll) * make_rotation_z(yaw);
  }

  if constexpr ((CONVENTION == RotationConvention::XZY) || (CONVENTION == RotationConvention::ROLL_YAW_PITCH))
  {
    return make_rotation_y(pitch) * make_rotation_z(yaw) * make_rotation_x(roll);
  }

  if constexpr ((CONVENTION == RotationConvention::ZYX) || (CONVENTION == RotationConvention::YAW_PITCH_ROLL))
  {
    return make_rotation_x(roll) * make_rotation_y(pitch) * make_rotation_z(yaw);
  }

  // (CONVENTION == RotationConvention::YXZ) || (CONVENTION == RotationConvention::PITCH_ROLL_YAW)
  return make_rotation_z(yaw) * make_rotation_x(roll) * make_rotation_y(pitch);
}

/// A 3D rotation matrix around given Euler angles in right-handed coordinate system.
/// The order of the rotations is determined by the rotation convention.
/// @tparam T The type of the elements.
/// @tparam CONVENTION The rotation convention to use for the order of the rotations.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @return The 3D rotation matrix.
template <typename T, RotationConvention CONVENTION>
constexpr Matrix3x3<T> make_rotation(const EulerAngles<T>& angles) noexcept
{
  return make_rotation<T, CONVENTION>(angles.roll, angles.pitch, angles.yaw);
}

/// A 3D rotation matrix around the x-, y- and z-axis in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_rotation(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw) noexcept
{
  return make_rotation<T, RotationConvention::ROLL_PITCH_YAW>(roll, pitch, yaw);
}

/// A 3D rotation matrix around the x-, y- and z-axis in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_rotation(const EulerAngles<T>& angles) noexcept
{
  return make_rotation(angles.roll, angles.pitch, angles.yaw);
}

/// An SO(3) rotation around the x-, y- and z-axis in right-handed 3D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_so3(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw) noexcept
{
  return make_rotation(roll, pitch, yaw);
}

/// An SO(3) rotation around the x-, y- and z-axis in right-handed 3D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @return The 3D rotation matrix.
template <typename T>
constexpr Matrix3x3<T> make_so3(const EulerAngles<T>& angles) noexcept
{
  return make_rotation(angles);
}

/// A 3D translation matrix.
/// @tparam T The type of the elements.
/// @param[in] tx The translation in the x direction.
/// @param[in] ty The translation in the y direction.
/// @param[in] tz The translation in the z direction.
/// @return The 3D translation matrix.
template <typename T>
constexpr Matrix4x4<T> make_translation(const T tx, const T ty, const T tz) noexcept
{
  // clang-format off
  return Matrix4x4<T>{
    T{1}, T{0}, T{0},   tx,
    T{0}, T{1}, T{0},   ty,
    T{0}, T{0}, T{1},   tz,
    T{0}, T{0}, T{0}, T{1},
  };
  // clang-format on
}

/// A 3D translation matrix.
/// @tparam T The type of the elements.
/// @param[in] translation The translation vector.
/// @return The 3D translation matrix.
template <typename T>
constexpr Matrix4x4<T> make_translation(const Vector3<T>& translation) noexcept
{
  return make_translation(translation.x(), translation.y(), translation.z());
}

/// A 3D, scale, rotation and translation matrix.
/// The matrix includes a scale, a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling matrix.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation matrix.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const Matrix3x3<T>& scale, const Matrix3x3<T>& rotation,
                                      const Matrix4x4<T>& translation) noexcept
{
  return translation * make_homogeneous(rotation * scale);
}

/// A 3D transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const Matrix3x3<T>& rotation, const Vector3<T>& translation) noexcept
{
  return make_transform(Matrix3x3<T>::identity(), rotation, make_translation(translation));
}

/// An SE(3) transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_se3(const Matrix3x3<T>& rotation, const Vector3<T>& translation) noexcept
{
  return make_transform(rotation, translation);
}

/// A 3D transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// The order of the rotations is determined by the rotation convention.
/// @tparam T The type of the elements.
/// @tparam CONVENTION The rotation convention to use for the order of the rotations.
/// @param[in] scale The scaling matrix.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T, RotationConvention CONVENTION>
constexpr Matrix4x4<T> make_transform(const Vector3<T>& scale, const Angle<T> roll, const Angle<T> pitch,
                                      const Angle<T> yaw, const Vector3<T>& translation) noexcept
{
  const auto rotation = make_rotation<T, CONVENTION>(roll, pitch, yaw);
  return make_transform(make_scale(scale), rotation, make_translation(translation));
}

/// A 3D transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling matrix.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const Vector3<T>& scale, const Angle<T> roll, const Angle<T> pitch,
                                      const Angle<T> yaw, const Vector3<T>& translation) noexcept
{
  return make_transform<T, RotationConvention::ROLL_PITCH_YAW>(scale, roll, pitch, yaw, translation);
}

/// A 3D transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling matrix.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const Vector3<T>& scale, const EulerAngles<T>& angles,
                                      const Vector3<T>& translation) noexcept
{
  return make_transform(scale, angles.roll, angles.pitch, angles.yaw, translation);
}

/// A 3D transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw,
                                      const Vector3<T>& translation) noexcept
{
  return make_transform(Vector3<T>{T{1}, T{1}, T{1}}, roll, pitch, yaw, translation);
}

/// A 3D transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_transform(const EulerAngles<T>& angles, const Vector3<T>& translation) noexcept
{
  return make_transform(angles.roll, angles.pitch, angles.yaw, translation);
}

/// An SE(3) transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] roll The angle of the rotation around the x-axis.
/// @param[in] pitch The angle of the rotation around the y-axis.
/// @param[in] yaw The angle of the rotation around the z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_se3(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw,
                                const Vector3<T>& translation) noexcept
{
  return make_transform(roll, pitch, yaw, translation);
}

/// An SE(3) transformation matrix.
/// The matrix includes a rotation (around x-, y- and z-axis) and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] angles The angles of the rotation around the x-, y- and z-axis.
/// @param[in] translation The translation vector.
/// @return The 3D rigid motion.
template <typename T>
constexpr Matrix4x4<T> make_se3(const EulerAngles<T>& angles, const Vector3<T>& translation) noexcept
{
  return make_transform(angles, translation);
}

/// Extract the rotation matrix from a 3D transformation matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The rotation matrix.
template <typename T>
constexpr Matrix3x3<T> rotation(const Matrix4x4<T>& matrix) noexcept
{
  // clang-format off
  return Matrix3x3<T>{
    matrix(0, 0), matrix(0, 1), matrix(0, 2),
    matrix(1, 0), matrix(1, 1), matrix(1, 2),
    matrix(2, 0), matrix(2, 1), matrix(2, 2),
  };
  // clang-format on
}

/// Extract the translation vector from a 3D transformation matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The translation vector.
template <typename T>
constexpr Vector3<T> translation(const Matrix4x4<T>& matrix) noexcept
{
  return Vector3<T>{matrix(0, 3), matrix(1, 3), matrix(2, 3)};
}

/// The inverse of a 3D rotation matrix.
/// Where R is a rotation matrix, R^T = R^-1.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix3x3<T> inverse_rotation(const Matrix3x3<T>& matrix) noexcept
{
  return transpose(matrix);
}

/// The inverse of an SO(3) rotation matrix.
/// Where R is a rotation matrix, R^T = R^-1.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix3x3<T> inverse_so3(const Matrix3x3<T>& matrix) noexcept
{
  return inverse_rotation(matrix);
}

/// The inverse of a 3D transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// Inversion is done by first inverting the rotation and then the translation, i.e.:
/// SE(2)^-1 = [
///   R^-1 -R^-1 * t
///      0         1
/// ].
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix4x4<T> inverse_transform(const Matrix4x4<T>& matrix) noexcept
{
  const auto inv_rotation = inverse_rotation(rotation(matrix));
  const auto inv_translation = -inv_rotation * translation(matrix);
  return make_transform(inv_rotation, inv_translation);
}

/// The inverse of an SE(3) transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// Inversion is done by first inverting the rotation and then the translation, i.e.:
/// SE(2)^-1 = [
///  R^-1 -R^-1 * t
///   0         1
///   ].
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix4x4<T> inverse_se3(const Matrix4x4<T>& matrix) noexcept
{
  return inverse_transform(matrix);
}

} // namespace rtw::math::transform3
