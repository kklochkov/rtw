#pragma once

#include "constants/math_constants.h"
#include "multiprecision/fixed_point.h"

#include <cmath>

namespace rtw::math
{

template <typename T>
class Angle;

template <typename T>
constexpr inline Angle<T> PI{math_constants::PI<T>};

template <typename T>
constexpr inline Angle<T> TAU = T{2} * PI<T>;

template <typename T>
constexpr inline Angle<T> PI_2 = PI<T> / T{2};

struct DegTag
{
  enum class Tag : std::uint8_t
  {
    TAG
  };

  constexpr explicit DegTag(Tag /*tag*/) noexcept {}
};
struct RadTag
{
  enum class Tag : std::uint8_t
  {
    TAG
  };

  constexpr explicit RadTag(Tag /*tag*/) noexcept {}
};

constexpr inline DegTag DEG{DegTag::Tag::TAG};
constexpr inline RadTag RAD{RadTag::Tag::TAG};

template <typename T>
class Angle
{
public:
  using value_type = T;

  static_assert(multiprecision::IS_ARITHMETIC_V<value_type>, "T must be arithmetic");

  constexpr Angle() noexcept = default;
  constexpr explicit Angle(const value_type value) noexcept : rad_(value) {}
  constexpr Angle(DegTag /*tag*/, const value_type value) noexcept
      : rad_(value * math_constants::DEG_TO_RAD<value_type>)
  {
  }
  constexpr Angle(RadTag /*tag*/, const value_type value) noexcept : Angle(value) {}

  constexpr value_type rad() const noexcept { return rad_; }
  constexpr value_type deg() const noexcept { return rad_ * math_constants::RAD_TO_DEG<value_type>; }
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator value_type() const noexcept { return rad_; }

  constexpr Angle operator-() const noexcept { return Angle{-rad_}; }

  constexpr Angle& operator+=(const Angle& other) noexcept
  {
    rad_ += other.rad();
    return *this;
  }

  constexpr Angle& operator-=(const Angle& other) noexcept
  {
    rad_ -= other.rad();
    return *this;
  }

  constexpr Angle& operator*=(const value_type scalar) noexcept
  {
    rad_ *= scalar;
    return *this;
  }

  constexpr Angle& operator/=(const value_type scalar) noexcept
  {
    rad_ /= scalar;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Angle operator+(const Angle& lhs, const Angle& rhs) noexcept { return Angle{lhs.rad() + rhs.rad()}; }
  friend constexpr Angle operator-(const Angle& lhs, const Angle& rhs) noexcept { return Angle{lhs.rad() - rhs.rad()}; }
  friend constexpr Angle operator*(const Angle& lhs, const value_type rhs) noexcept { return Angle{lhs.rad() * rhs}; }
  friend constexpr Angle operator*(const value_type lhs, const Angle& rhs) noexcept { return Angle{lhs * rhs.rad()}; }
  friend constexpr Angle operator/(const Angle& lhs, const value_type rhs) noexcept { return Angle{lhs.rad() / rhs}; }
  friend constexpr Angle operator/(const value_type lhs, const Angle& rhs) noexcept { return Angle{lhs / rhs.rad()}; }
  friend constexpr bool operator==(const Angle& lhs, const Angle& rhs) noexcept
  {
    if constexpr (std::is_floating_point_v<value_type>)
    {
      return is_near_equal(lhs.rad(), rhs.rad()) || is_near_equal(lhs.rad(), rhs.rad() + TAU<value_type>)
          || is_near_equal(lhs.rad(), rhs.rad() - TAU<value_type>);
    }
    else
    {
      return lhs.rad() == rhs.rad()
          || lhs.rad() == rhs.rad() + TAU<value_type> || lhs.rad() == rhs.rad() - TAU<value_type>;
    }
  }
  friend constexpr bool operator!=(const Angle& lhs, const Angle& rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const Angle& lhs, const Angle& rhs) noexcept { return lhs.rad() < rhs.rad(); }
  friend constexpr bool operator>(const Angle& lhs, const Angle& rhs) noexcept { return lhs.rad() > rhs.rad(); }
  friend constexpr bool operator<=(const Angle& lhs, const Angle& rhs) noexcept { return lhs.rad() <= rhs.rad(); }
  friend constexpr bool operator>=(const Angle& lhs, const Angle& rhs) noexcept { return lhs.rad() >= rhs.rad(); }
  /// @}

private:
  value_type rad_{0};
};

using AngleF = Angle<float>;
using AngleD = Angle<double>;
using AngleQ16 = Angle<multiprecision::FixedPoint16>;
using AngleQ32 = Angle<multiprecision::FixedPoint32>;

template <typename T>
Angle<T> normalize(const Angle<T>& angle) noexcept
{
  // std::fmod is not constexpr in C++17, but it is in C++20.
  Angle<T> result{std::fmod(angle + PI<T>, TAU<T>)};
  if (result < 0)
  {
    result += TAU<T>;
  }
  return result - PI<T>;
}

template <typename T>
T distance(const Angle<T>& lhs, const Angle<T>& rhs) noexcept
{
  return normalize(rhs - lhs);
}

template <typename T>
Angle<T> interpolate(const Angle<T>& lhs, const Angle<T>& rhs, const T t) noexcept
{
  return normalize(Angle<T>{((T{1} - t) * lhs) + (t * rhs)});
}

namespace angle_literals
{

constexpr AngleD operator""_degD(const long double value) noexcept { return AngleD{DEG, static_cast<double>(value)}; }

constexpr AngleD operator""_radD(const long double value) noexcept { return AngleD{RAD, static_cast<double>(value)}; }

constexpr AngleF operator""_degF(const long double value) noexcept { return AngleF{DEG, static_cast<float>(value)}; }

constexpr AngleF operator""_radF(const long double value) noexcept { return AngleF{RAD, static_cast<float>(value)}; }

constexpr AngleQ16 operator""_degQ16(const long double value) noexcept
{
  return AngleQ16{DEG, static_cast<float>(value)};
}

constexpr AngleQ32 operator""_radQ32(const long double value) noexcept
{
  return AngleQ32{RAD, static_cast<float>(value)};
}

} // namespace angle_literals

/// A set of not proper (classic) Euler angles.
/// The order of the rotations is yaw-pitch-roll.
/// The cooridate system is right-handed.
/// @tparam T The type of the elements.
template <typename T>
struct EulerAngles
{
  constexpr Angle<T>& z() noexcept { return yaw; }
  constexpr const Angle<T>& z() const noexcept { return yaw; }

  constexpr Angle<T>& y() noexcept { return pitch; }
  constexpr const Angle<T>& y() const noexcept { return pitch; }

  constexpr Angle<T>& x() noexcept { return roll; }
  constexpr const Angle<T>& x() const noexcept { return roll; }

  constexpr Angle<T>& alpha() noexcept { return yaw; }
  constexpr const Angle<T>& alpha() const noexcept { return yaw; }

  constexpr Angle<T>& beta() noexcept { return pitch; }
  constexpr const Angle<T>& beta() const noexcept { return pitch; }

  constexpr Angle<T>& gamma() noexcept { return roll; }
  constexpr const Angle<T>& gamma() const noexcept { return roll; }

  constexpr Angle<T>& psi() noexcept { return yaw; }
  constexpr const Angle<T>& psi() const noexcept { return yaw; }

  constexpr Angle<T>& theta() noexcept { return pitch; }
  constexpr const Angle<T>& theta() const noexcept { return pitch; }

  constexpr Angle<T>& phi() noexcept { return roll; }
  constexpr const Angle<T>& phi() const noexcept { return roll; }

  Angle<T> yaw;
  Angle<T> pitch;
  Angle<T> roll;
};

enum class RotationConvention : std::uint8_t
{
  XYZ = 0U, // regular Euler angles
  // Tait-Bryan angles
  YZX = 1U, // common in game engines
  ZXY = 2U,
  XZY = 3U,
  ZYX = 4U, // common in aerospace and robotics
  YXZ = 5U,
  // Aliases
  ROLL_PITCH_YAW = XYZ,
  PITCH_YAW_ROLL = YZX,
  YAW_ROLL_PITCH = ZXY,
  ROLL_YAW_PITCH = XZY,
  YAW_PITCH_ROLL = ZYX,
  PITCH_ROLL_YAW = YXZ
};

using EulerAnglesF = EulerAngles<float>;
using EulerAnglesD = EulerAngles<double>;
using EulerAnglesQ16 = EulerAngles<multiprecision::FixedPoint16>;
using EulerAnglesQ32 = EulerAngles<multiprecision::FixedPoint32>;

} // namespace rtw::math
