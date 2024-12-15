#pragma once

#include <cmath>
#include <type_traits>

namespace rtw::math
{

template <typename T>
class Angle;

template <typename T>
constexpr Angle<T> PI{3.141592653589793238462643383279502884197169399375105820974944592307816406286};

template <typename T>
constexpr Angle<T> TAO = T{2} * PI<T>;

template <typename T>
constexpr Angle<T> PI_2 = PI<T> / T{2};

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

constexpr static DegTag DEG{DegTag::Tag::TAG};
constexpr static RadTag RAD{RadTag::Tag::TAG};

template <typename T>
class Angle
{
public:
  using value_type = T;

  static_assert(std::is_arithmetic_v<value_type>, "T must be arithmetic");

  constexpr Angle() noexcept = default;
  constexpr explicit Angle(const value_type value) noexcept : rad_(value) {}
  constexpr Angle(DegTag /*tag*/, const value_type value) noexcept : rad_(value * PI<value_type> / value_type{180}) {}
  constexpr Angle(RadTag /*tag*/, const value_type value) noexcept : Angle(value) {}

  constexpr value_type rad() const noexcept { return rad_; }
  constexpr value_type deg() const noexcept { return rad_ * value_type{180} / PI<value_type>; }
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
    // TODO: Floating point comparison is hard.
    return lhs.rad() == rhs.rad()
        || lhs.rad() == rhs.rad() + TAO<value_type> || lhs.rad() == rhs.rad() - TAO<value_type>;
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

using Anglef = Angle<float>;
using Angled = Angle<double>;

template <typename T>
Angle<T> normalize(const Angle<T>& angle) noexcept
{
  // std::fmod is not constexpr in C++17, but it is in C++20.
  Angle<T> result{std::fmod(angle + PI<T>, TAO<T>)};
  if (result < 0)
  {
    result += TAO<T>;
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
  return normalize(Angle<T>{(T{1} - t) * lhs + t * rhs});
}

namespace angle_literals
{

constexpr Angled operator""_deg(const long double value) noexcept { return Angled{DEG, static_cast<double>(value)}; }

constexpr Angled operator""_rad(const long double value) noexcept { return Angled{RAD, static_cast<double>(value)}; }

constexpr Anglef operator""_degf(const long double value) noexcept { return Anglef{DEG, static_cast<float>(value)}; }

constexpr Anglef operator""_radf(const long double value) noexcept { return Anglef{RAD, static_cast<float>(value)}; }

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

using EulerAnglesf = EulerAngles<float>;
using EulerAnglesd = EulerAngles<double>;

} // namespace rtw::math
