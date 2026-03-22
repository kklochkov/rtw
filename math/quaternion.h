#pragma once

#include "math/angle.h"
#include "math/vector.h"

namespace rtw::math
{

template <typename T>
class Quaternion : Matrix<T, 4, 1>
{
public:
  using Matrix = Matrix<T, 4, 1>;
  using Vector = Vector<T, 4>;

  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::operator+=;
  using Matrix::operator-=;
  using Matrix::operator*=;
  using Matrix::operator/=;
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;
  using Matrix::size;
  using Matrix::w;
  using Matrix::x;
  using Matrix::y;
  using Matrix::z;

  constexpr Quaternion() noexcept = default;

  constexpr explicit Quaternion(UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr Quaternion(InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}
  constexpr explicit Quaternion(IdentityTag /*tag*/) noexcept : Quaternion(T{1}, T{0}, T{0}, T{0}) {}
  constexpr explicit Quaternion(ZeroTag /*tag*/) noexcept : Quaternion(T{0}, T{0}, T{0}, T{0}) {}

  constexpr Quaternion(const T w, const T x, const T y, const T z) noexcept : Matrix{x, y, z, w} {}
  template <MemoryOrder MEMORY_ORDER>
  constexpr explicit Quaternion(const math::Matrix<T, 4, 1, MEMORY_ORDER>& matrix) noexcept
      : Quaternion(matrix.w(), matrix.x(), matrix.y(), matrix.z())
  {
  }
  constexpr explicit Quaternion(const Vector& vector) noexcept
      : Quaternion(vector.w(), vector.x(), vector.y(), vector.z())
  {
  }

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const noexcept { return Vector(as_matrix()); }
  constexpr explicit operator Matrix() const noexcept { return as_matrix(); }

  template <typename U = value_type, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<U>>>
  constexpr Quaternion<U> cast() const noexcept
  {
    return Quaternion<U>{as_matrix().template cast<U>()};
  }

  constexpr math::Vector<value_type, 3> xyz() const noexcept { return math::Vector<value_type, 3>{as_matrix().xyz()}; }

  constexpr Quaternion conjugate() const noexcept { return {w(), -x(), -y(), -z()}; }

  constexpr Angle<T> angle() const noexcept
  {
    using multiprecision::math::acos;
    using multiprecision::math::clamp;
    using std::acos;
    using std::clamp;

    return Angle<T>{T{2} * acos(clamp(w(), T{-1}, T{1}))};
  }

  constexpr static Quaternion from_axis_angle(const Vector3<T>& axis, const Angle<T> angle) noexcept
  {
    using multiprecision::math::cos;
    using multiprecision::math::sin;
    using std::cos;
    using std::sin;

    const auto half_angle = angle / T{2};
    const auto sin_half_angle = sin(half_angle);
    return {cos(half_angle), axis.x() * sin_half_angle, axis.y() * sin_half_angle, axis.z() * sin_half_angle};
  }

  constexpr static Quaternion from_euler_angles(const Angle<T> roll, const Angle<T> pitch, const Angle<T> yaw) noexcept
  {
    using multiprecision::math::cos;
    using multiprecision::math::sin;
    using std::cos;
    using std::sin;

    const auto half_roll = roll / T{2};
    const auto half_pitch = pitch / T{2};
    const auto half_yaw = yaw / T{2};

    const auto cr = cos(half_roll);
    const auto sr = sin(half_roll);
    const auto cp = cos(half_pitch);
    const auto sp = sin(half_pitch);
    const auto cy = cos(half_yaw);
    const auto sy = sin(half_yaw);

    return {
        (cr * cp * cy) + (sr * sp * sy), // w
        (sr * cp * cy) - (cr * sp * sy), // x
        (cr * sp * cy) + (sr * cp * sy), // y
        (cr * cp * sy) - (sr * sp * cy)  // z
    };
  }

  constexpr static Quaternion from_euler_angles(const EulerAngles<T>& euler_angles) noexcept
  {
    return from_euler_angles(euler_angles.roll, euler_angles.pitch, euler_angles.yaw);
  }

  constexpr EulerAngles<T> to_euler_angles() const noexcept
  {
    using multiprecision::math::asin;
    using multiprecision::math::atan2;
    using multiprecision::math::clamp;
    using std::asin;
    using std::atan2;
    using std::clamp;

    const Angle<T> roll{atan2(T{2} * (w() * x() + y() * z()), T{1} - T{2} * (x() * x() + y() * y()))};
    const Angle<T> pitch{asin(clamp(T{2} * (w() * y() - z() * x()), T{-1}, T{1}))};
    const Angle<T> yaw{atan2(T{2} * (w() * z() + x() * y()), T{1} - T{2} * (y() * y() + z() * z()))};
    return {yaw, pitch, roll};
  }

  template <MemoryOrder MEMORY_ORDER = DEFAULT_MEMORY_ORDER>
  constexpr static Quaternion from_rotation_matrix(const Matrix3x3<T, MEMORY_ORDER>& rotation_matrix) noexcept
  {
    using multiprecision::math::sqrt;
    using std::sqrt;

    const auto trace = rotation_matrix(0, 0) + rotation_matrix(1, 1) + rotation_matrix(2, 2);
    if (trace > T{0})
    {
      const auto s = sqrt(trace + T{1}) * T{2};
      return {s / T{4}, (rotation_matrix(2, 1) - rotation_matrix(1, 2)) / s,
              (rotation_matrix(0, 2) - rotation_matrix(2, 0)) / s, (rotation_matrix(1, 0) - rotation_matrix(0, 1)) / s};
    }

    if ((rotation_matrix(0, 0) > rotation_matrix(1, 1)) && (rotation_matrix(0, 0) > rotation_matrix(2, 2)))
    {
      const auto s = sqrt(T{1} + rotation_matrix(0, 0) - rotation_matrix(1, 1) - rotation_matrix(2, 2)) * T{2};
      return {(rotation_matrix(2, 1) - rotation_matrix(1, 2)) / s, s / T{4},
              (rotation_matrix(0, 1) + rotation_matrix(1, 0)) / s, (rotation_matrix(0, 2) + rotation_matrix(2, 0)) / s};
    }

    if (rotation_matrix(1, 1) > rotation_matrix(2, 2))
    {
      const auto s = sqrt(T{1} + rotation_matrix(1, 1) - rotation_matrix(0, 0) - rotation_matrix(2, 2)) * T{2};
      return {(rotation_matrix(0, 2) - rotation_matrix(2, 0)) / s, (rotation_matrix(0, 1) + rotation_matrix(1, 0)) / s,
              s / T{4}, (rotation_matrix(1, 2) + rotation_matrix(2, 1)) / s};
    }

    const auto s = sqrt(T{1} + rotation_matrix(2, 2) - rotation_matrix(0, 0) - rotation_matrix(1, 1)) * T{2};
    return {(rotation_matrix(1, 0) - rotation_matrix(0, 1)) / s, (rotation_matrix(0, 2) + rotation_matrix(2, 0)) / s,
            (rotation_matrix(1, 2) + rotation_matrix(2, 1)) / s, s / T{4}};
  }

  template <MemoryOrder MEMORY_ORDER = DEFAULT_MEMORY_ORDER>
  constexpr Matrix3x3<T, MEMORY_ORDER> to_rotation_matrix() const noexcept
  {
    const auto xx = x() * x();
    const auto yy = y() * y();
    const auto zz = z() * z();
    const auto xy = x() * y();
    const auto xz = x() * z();
    const auto yz = y() * z();
    const auto wx = w() * x();
    const auto wy = w() * y();
    const auto wz = w() * z();

    // clang-format off
    return Matrix3x3<T, MEMORY_ORDER>{
        T{1} - T{2} * (yy + zz),        T{2} * (xy - wz),        T{2} * (xz + wy),
               T{2} * (xy + wz), T{1} - T{2} * (xx + zz),        T{2} * (yz - wx),
               T{2} * (xz - wy),        T{2} * (yz + wx), T{1} - T{2} * (xx + yy),
    };
    // clang-format on
  }

  constexpr Quaternion& operator*=(const Quaternion& rhs) noexcept
  {
    // Hamilton product of two quaternions.
    const auto new_w = (w() * rhs.w()) - (x() * rhs.x()) - (y() * rhs.y()) - (z() * rhs.z());
    const auto new_x = (w() * rhs.x()) + (x() * rhs.w()) + (y() * rhs.z()) - (z() * rhs.y());
    const auto new_y = (w() * rhs.y()) - (x() * rhs.z()) + (y() * rhs.w()) + (z() * rhs.x());
    const auto new_z = (w() * rhs.z()) + (x() * rhs.y()) - (y() * rhs.x()) + (z() * rhs.w());
    w() = new_w;
    x() = new_x;
    y() = new_y;
    z() = new_z;
    return *this;
  }

  constexpr Quaternion& operator*=(const value_type rhs) noexcept
  {
    as_matrix() *= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Quaternion operator*(Quaternion lhs, const Quaternion& rhs) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  friend constexpr Vector3<T> operator*(const Quaternion& quaternion, const Vector3<T>& vector) noexcept
  {
    // Rotate the vector using v' = q * v * q^-1, where v is treated as pure/vector quaternion (w = 0).
    const Quaternion p{T{0}, vector.x(), vector.y(), vector.z()};
    const Quaternion q_conjugate = quaternion.conjugate();
    const Quaternion result = quaternion * p * q_conjugate;
    return result.xyz();
  }

  friend constexpr Quaternion operator*(const Quaternion& lhs, const value_type rhs) noexcept
  {
    return Quaternion{lhs.as_matrix() * rhs};
  }

  friend constexpr Quaternion operator*(const value_type lhs, const Quaternion& rhs) noexcept { return rhs * lhs; }

  friend constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs) noexcept
  {
    return Quaternion{lhs.as_matrix() + rhs.as_matrix()};
  }

  friend constexpr Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs) noexcept
  {
    return Quaternion{lhs.as_matrix() - rhs.as_matrix()};
  }

  friend constexpr Quaternion operator/(const Quaternion& lhs, const value_type rhs) noexcept
  {
    return Quaternion{lhs.as_matrix() / rhs};
  }

  friend constexpr Quaternion operator-(const Quaternion& quaternion) noexcept
  {
    return Quaternion{-quaternion.as_matrix()};
  }

  friend constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs) noexcept { return !(lhs == rhs); }
  /// @}
};

} // namespace rtw::math
