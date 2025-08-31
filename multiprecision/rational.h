#pragma once

#include "multiprecision/int128.h"
#include "multiprecision/math.h"

#include <cmath>
#include <type_traits>

namespace rtw::multiprecision
{

template <typename T>
class Rational
{
public:
  static_assert((std::is_integral_v<T> && std::is_signed_v<T> && !std::is_floating_point_v<T>)
                    || IS_SIGNED_BIG_INT_V<T>,
                "Template parameter T must be a signed integer type or a signed big integer type.");

  constexpr Rational(const T numerator, const T denominator) noexcept : numerator_{numerator}, denominator_{denominator}
  {
    normalize();
  }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr Rational(const I numerator) noexcept : numerator_{numerator}
  {
  }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr Rational(const F value) noexcept
  {
    if (std::isnan(value) || std::isinf(value))
    {
      assert(false && "Cannot convert NaN or Inf to Rational");
      return;
    }

    if (value == 0.0)
    {
      return;
    }

    std::int32_t exponent{};
    const auto mantissa = std::frexp(value, &exponent); // value = mantissa * 2^exponent, where 0.5 <= |mantissa| < 1
    const auto mantissa_digits = std::numeric_limits<F>::digits;

    numerator_ = static_cast<T>(std::ldexp(mantissa, mantissa_digits)); // round(mantissa * 2^digits)
    denominator_ = static_cast<T>(T{1} << mantissa_digits);             // 2^digits

    if (exponent > 0)
    {
      numerator_ <<= exponent;
    }
    else
    {
      denominator_ <<= -exponent;
    }

    normalize();
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr T numerator() const noexcept { return numerator_; }
  constexpr T denominator() const noexcept { return denominator_; }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr explicit operator F() const noexcept
  {
    assert(denominator_ != 0 && "Denominator must not be zero");
    return static_cast<F>(numerator_) / static_cast<F>(denominator_);
  }

  constexpr Rational& operator+=(const Rational other) noexcept
  {
    // a/b + c/d = (a*d + b*c) / (b*d)
    numerator_ = (numerator_ * other.denominator_) + (other.numerator_ * denominator_);
    denominator_ *= other.denominator_;
    normalize();
    return *this;
  }

  constexpr Rational& operator-=(const Rational other) noexcept
  {
    // a/b - c/d = (a*d - b*c) / (b*d)
    numerator_ = (numerator_ * other.denominator_) - (other.numerator_ * denominator_);
    denominator_ *= other.denominator_;
    normalize();
    return *this;
  }

  constexpr Rational& operator*=(const Rational other) noexcept
  {
    // a/b * c/d = (a*c) / (b*d)
    numerator_ *= other.numerator_;
    denominator_ *= other.denominator_;
    normalize();
    return *this;
  }

  constexpr Rational& operator/=(const Rational other) noexcept
  {
    // a/b / c/d = (a*d) / (b*c)
    numerator_ *= other.denominator_;
    denominator_ *= other.numerator_;
    normalize();
    return *this;
  }

  constexpr Rational& operator++() noexcept
  {
    numerator_ += denominator_;
    return *this;
  }

  constexpr Rational operator++(int) noexcept
  {
    const auto result = *this;
    ++(*this);
    return result;
  }

  constexpr Rational& operator--() noexcept
  {
    numerator_ -= denominator_;
    return *this;
  }

  constexpr Rational operator--(int) noexcept
  {
    const auto result = *this;
    --(*this);
    return result;
  }

  constexpr Rational operator+() const noexcept { return *this; }
  constexpr Rational operator-() const noexcept { return Rational{-numerator_, denominator_}; }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Rational operator+(Rational lhs, const Rational rhs) noexcept { return lhs += rhs; }
  friend constexpr Rational operator-(Rational lhs, const Rational rhs) noexcept { return lhs -= rhs; }
  friend constexpr Rational operator*(Rational lhs, const Rational rhs) noexcept { return lhs *= rhs; }
  friend constexpr Rational operator/(Rational lhs, const Rational rhs) noexcept { return lhs /= rhs; }
  /// @}

  /// Comparison operators
  /// @{
  friend constexpr bool operator==(const Rational lhs, const Rational rhs) noexcept
  {
    return (lhs.numerator_ == rhs.numerator_) && (lhs.denominator_ == rhs.denominator_);
  }
  friend constexpr bool operator!=(const Rational lhs, const Rational rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const Rational lhs, const Rational rhs) noexcept
  {
    return (lhs.numerator_ * rhs.denominator_) < (rhs.numerator_ * lhs.denominator_);
  }
  friend constexpr bool operator>(const Rational lhs, const Rational rhs) noexcept { return rhs < lhs; }
  friend constexpr bool operator<=(const Rational lhs, const Rational rhs) noexcept { return !(lhs > rhs); }
  friend constexpr bool operator>=(const Rational lhs, const Rational rhs) noexcept { return !(lhs < rhs); }
  /// @}

private:
  constexpr void normalize() noexcept
  {
    assert(denominator_ != 0 && "Denominator must not be zero");

    if (denominator_ < 0)
    {
      numerator_ = -numerator_;
      denominator_ = -denominator_;
    }

    if (numerator_ == 0)
    {
      denominator_ = 1;
      return;
    }

    using math::abs;
    using std::abs;

    const T gcd_value = gcd(abs(numerator_), denominator_);
    numerator_ /= gcd_value;
    denominator_ /= gcd_value;
  }

  constexpr static T gcd(T a, T b) noexcept
  {
    while (b != 0)
    {
      const T rem = a % b;
      a = b;
      b = rem;
    }
    return a;
  }

  T numerator_{0};
  T denominator_{1};
};

using Rational32 = Rational<std::int32_t>;
using Rational64 = Rational<std::int64_t>;

} // namespace rtw::multiprecision
