#pragma once

#include "math/angle.h"
#include "math/matrix.h"
#include "math/vector.h"

namespace rtw::math::transform2 {

/// Make a homogeneous 2D matrix from a 2D matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The 2D matrix.
/// @return The homogeneous 2D matrix.
template <typename T>
constexpr Matrix3x3<T> make_homogeneous(const Matrix2x2<T>& matrix)
{
  // clang-format off
  return Matrix3x3<T>{
    matrix(0, 0), matrix(0, 1), T{0},
    matrix(1, 0), matrix(1, 1), T{0},
            T{0},         T{0}, T{1},
  };
  // clang-format on
}

/// A 2D scaling matrix.
/// @tparam T The type of the elements.
/// @param[in] sx The scaling factor in the x direction.
/// @param[in] sy The scaling factor in the y direction.
/// @return The 2D scaling matrix.
template <typename T>
constexpr Matrix2x2<T> make_scale(const T sx, const T sy)
{
  // clang-format off
    return Matrix2x2<T>{
      sx, T{0},
    T{0},   sy,
    };
  // clang-format on
}

/// A 2D scaling matrix.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling vector.
/// @return The 2D scaling matrix.
template <typename T>
constexpr Matrix2x2<T> make_scale(const Vector2<T>& scale)
{
  return make_scale(scale.x(), scale.y());
}

/// A 2D rotation matrix in right-handed coordinate system.
/// @tparam T The type of the elements.
/// @param[in] angle The angle of the rotation.
/// @return The 2D rotation matrix.
template <typename T>
constexpr Matrix2x2<T> make_rotation(const Angle<T> angle)
{
  const auto c = std::cos(angle);
  const auto s = std::sin(angle);
  // clang-format off
  return Matrix2x2<T>{
    c, -s,
    s,  c,
  };
  // clang-format on
}

/// An SO(2) rotation in right-handed 2D coordinate system.
/// @tparam T The type of the elements.
/// @param[in] angle The angle of the rotation.
/// @return The SO(2) rotation.
template <typename T>
constexpr Matrix2x2<T> make_so2(const Angle<T> angle)
{
  return make_rotation(angle);
}

/// A 2D translation matrix in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] tx The translation in the x direction.
/// @param[in] ty The translation in the y direction.
/// @return The 2D translation matrix.
template <typename T>
constexpr Matrix3x3<T> make_translation(const T tx, const T ty)
{
  // clang-format off
  return Matrix3x3<T>{
    T{1}, T{0},   tx,
    T{0}, T{1},   ty,
    T{0}, T{0}, T{1},
  };
  // clang-format on
}

/// A 2D translation matrix in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] translation The translation vector.
/// @return The 2D translation matrix.
template <typename T>
constexpr Matrix3x3<T> make_translation(const Vector2<T>& translation)
{
  return make_translation(translation.x(), translation.y());
}

/// A 2D transformation matrix.
/// The matrix includes a scale, a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling matrix.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation matrix.
/// @return The 2D rigid motion.
template <typename T>
constexpr Matrix3x3<T> make_transform(const Matrix2x2<T>& scale, const Matrix2x2<T>& rotation,
                                      const Matrix3x3<T>& translation)
{
  return translation * make_homogeneous(rotation * scale);
}

/// A 2D  transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation vector.
/// @return The 2D rigid motion.
template <typename T>
constexpr Matrix3x3<T> make_transform(const Matrix2x2<T>& rotation, const Matrix3x3<T>& translation)
{
  return make_transform(Matrix2x2<T>::identity(), rotation, translation);
}

/// A 2D transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] rotation The rotation matrix.
/// @param[in] translation The translation vector.
/// @return The 2D rigid motion.
template <typename T>
constexpr Matrix3x3<T> make_transform(const Matrix2x2<T>& rotation, const Vector2<T>& translation)
{
  return make_transform(rotation, make_translation(translation));
}

/// A 2D transformation matrix.
/// The matrix includes a scale and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] scale The scaling vector.
/// @param[in] translation The translation vector.
/// @return The 2D rigid motion.
template <typename T>
constexpr Matrix3x3<T> make_transform(const Vector2<T>& scale, const Vector2<T>& translation)
{
  return make_transform(make_scale(scale), Matrix2x2<T>::identity(), make_translation(translation));
}

/// A 2D transformation matrix.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] angle The angle of the rotation.
/// @param[in] translation The translation vector.
/// @return The 2D rigid motion.
template <typename T>
constexpr Matrix3x3<T> make_transform(const Angle<T> angle, const Vector2<T>& translation)
{
  return make_transform(make_rotation(angle), translation);
}

/// An SE(2) transformation in 2D.
/// The matrix includes a rotation and a translation in homogeneous coordinates.
/// @tparam T The type of the elements.
/// @param[in] angle The angle of the rotation.
/// @param[in] translation The translation vector.
/// @return The SE(2) transformation.
template <typename T>
constexpr Matrix3x3<T> make_se2(const Angle<T> angle, const Vector2<T>& translation)
{
  return make_transform(angle, translation);
}

/// Extract the translation vector from a 2D transformation matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The translation vector.
template <typename T>
constexpr Matrix2x2<T> rotation(const Matrix3x3<T>& matrix)
{
  // clang-format off
  return Matrix2x2<T>{
    matrix(0, 0), matrix(0, 1),
    matrix(1, 0), matrix(1, 1),
  };
  // clang-format on
}

/// Extract the translation vector from a 2D transformation matrix.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The translation vector.
template <typename T>
constexpr Vector2<T> translation(const Matrix3x3<T>& matrix)
{
  return Vector2<T>{matrix(0, 2), matrix(1, 2)};
}

/// The inverse of a 2D rotation matrix.
/// Where R is a rotation matrix, R^T = R^-1.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix2x2<T> inverse_rotation(const Matrix2x2<T>& matrix)
{
  return transpose(matrix);
}

/// The inverse of an SO(2) rotation matrix.
/// Where R is a rotation matrix, R^T = R^-1.
/// @tparam T The type of the elements.
/// @param[in] matrix The transformation matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix2x2<T> inverse_so2(const Matrix2x2<T>& matrix)
{
  return inverse_rotation(matrix);
}

/// The inverse of a 2D transformation matrix.
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
constexpr Matrix3x3<T> inverse_transform(const Matrix3x3<T>& matrix)
{
  const auto inv_rotation = inverse_rotation(rotation(matrix));
  const auto inv_translation = -inv_rotation * translation(matrix);
  return make_transform(inv_rotation, inv_translation);
}

/// The inverse of an SE(2) transformation in 2D.
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
constexpr Matrix3x3<T> inverse_se2(const Matrix3x3<T>& matrix)
{
  return inverse_transform(matrix);
}

} // namespace rtw::math::transform2
