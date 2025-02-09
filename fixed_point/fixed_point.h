#pragma once

#include "constants/math_constants.h"
#include "fixed_point/int128.h"
#include "fixed_point/operations.h"

#include <algorithm>
#include <cstdint>
#include <iostream>

namespace rtw::fixed_point
{

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
class FixedPoint;

namespace math
{

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> abs(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> floor(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> ceil(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> round(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sqrt(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> sin(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
constexpr FixedPoint<T, FRAC_BITS, SaturationT> cos(const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept;

} // namespace math

/// Fixed-point number representation using QM.N format (ARM notation).
/// Overflows are handled by saturating the result to the minimum or maximum value.
/// See https://en.wikipedia.org/wiki/Q_(number_format) for more information.
/// More information about fixed-point arithmetic can be found here:
/// https://en.m.wikipedia.org/wiki/Fixed-point_arithmetic
/// Ranges are:
/// - Signed: -2^(M-1) to 2^(M-1) - 2^(-N)
/// - Unsigned: 0 to 2^(M-1) - 2^(-N)
/// @note The saturation of the unsigned fixed-point number might yield wrap-around behavior in some cases.
/// 1. Case: when subtracting a larger number from a smaller number, the result will wrap around to the maximum value.
/// For example, `c = 0 - 1 = max_unsigned - 1`.
/// 2. Case: when constructing a fixed-point number from a negative floating-point number,
/// after rescaling and rounding, the result will be 0.
/// I.e., `cast_to_unsigned(round(value * ONE)) * RESOLUTION = max_unsigned - 1`.
/// In fact the latter case yields the same result as saturation to minimum value.
/// @tparam T The underlying type of the fixed-point number.
/// @tparam FRAC_BITS The number of fractional bits.
/// @tparam SaturationT The type used for saturation.
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
class FixedPoint
{
  // clang-format off
  struct PrivateCtorTag {};
  // clang-format on
  constexpr static PrivateCtorTag PRIVATE_CTOR = {};
  constexpr FixedPoint(PrivateCtorTag /*tag*/, const T value) noexcept : value_(value) {}

  constexpr static T saturate_and_cast(const SaturationT value) noexcept
  {
    return static_cast<T>(
        std::clamp(value, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER)));
  }

public:
  using underlying_type = T;
  using saturation_type = SaturationT;

  constexpr static std::uint32_t BITS = sizeof(T) * 8U;
  constexpr static std::uint32_t FRACTIONAL_BITS = FRAC_BITS;
  constexpr static std::uint32_t INTEGER_BITS = BITS - FRACTIONAL_BITS - std::uint32_t{std::is_signed_v<T>};
  constexpr static T ONE = 1UL << FRACTIONAL_BITS;
  constexpr static T HALF = ONE >> 1U;
  constexpr static double RESOLUTION = 1.0L / static_cast<double>(ONE);
  constexpr static T MAX_INTEGER = std::numeric_limits<T>::max();
  constexpr static T MIN_INTEGER = std::numeric_limits<T>::min();
  constexpr static T FRACTION_MASK = ONE - 1U;
  constexpr static T INTEGER_MASK = ~FRACTION_MASK;
  constexpr static T PI_INTEGER = round_to_nearest_integer(math_constants::PI<double> * ONE);
  constexpr static T PI_2_INTEGER = round_to_nearest_integer(math_constants::PI_2<double> * ONE);
  constexpr static T PI_4_INTEGER = round_to_nearest_integer(math_constants::PI_4<double> * ONE);
  constexpr static T TWO_PI_INTEGER = round_to_nearest_integer(math_constants::TWO_PI<double> * ONE);

  static_assert(FRACTIONAL_BITS < BITS, "The number of fractional bits must be less than the total bits");
  static_assert(std::is_integral_v<T>, "The underlying type must be an integral type");
  static_assert((std::is_signed_v<SaturationT> == std::is_signed_v<T>) || (std::is_same_v<SaturationT, Int128>),
                "The saturation type must have the same sign as the underlying type");

  constexpr FixedPoint() noexcept = default;

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr FixedPoint(const I value) noexcept
  {
    const auto result = static_cast<SaturationT>(value) << FRACTIONAL_BITS;
    value_ = saturate_and_cast(result);
  }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr FixedPoint(const F value) noexcept
  {
    const auto result = static_cast<SaturationT>(round_to_nearest_integer(value * ONE));
    value_ = saturate_and_cast(result);
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr static FixedPoint min() noexcept { return FixedPoint(PRIVATE_CTOR, MIN_INTEGER); }
  constexpr static FixedPoint max() noexcept { return FixedPoint(PRIVATE_CTOR, MAX_INTEGER); }

  /// Constants
  /// @{
  constexpr static FixedPoint pi() noexcept { return FixedPoint(PRIVATE_CTOR, PI_INTEGER); }
  constexpr static FixedPoint pi_2() noexcept { return FixedPoint(PRIVATE_CTOR, PI_2_INTEGER); }
  constexpr static FixedPoint pi_4() noexcept { return FixedPoint(PRIVATE_CTOR, PI_4_INTEGER); }
  constexpr static FixedPoint two_pi() noexcept { return FixedPoint(PRIVATE_CTOR, TWO_PI_INTEGER); }
  constexpr static FixedPoint tau() noexcept { return two_pi(); }
  /// @}

  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr explicit operator I() const noexcept
  {
    return static_cast<I>(value_ >> FRACTIONAL_BITS);
  }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr explicit operator F() const noexcept
  {
    return static_cast<F>(value_) * static_cast<F>(RESOLUTION);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_signed_v<U>>>
  constexpr FixedPoint operator-() const noexcept
  {
    return FixedPoint(PRIVATE_CTOR, -value_);
  }

  constexpr FixedPoint& operator+=(const FixedPoint rhs) noexcept
  {
    const auto result = static_cast<SaturationT>(value_) + static_cast<SaturationT>(rhs.value_);
    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator-=(const FixedPoint rhs) noexcept
  {
    const auto result = static_cast<SaturationT>(value_) - static_cast<SaturationT>(rhs.value_);
    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator*=(const FixedPoint rhs) noexcept
  {
    auto result = static_cast<SaturationT>(value_) * static_cast<SaturationT>(rhs.value_);

    // Rounding, if the result is negative, add 1/2 to the result
    result += (1U << (FRACTIONAL_BITS - 1U));

    // Scale back to the original number of fractional bits
    result >>= FRACTIONAL_BITS;

    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator/=(const FixedPoint rhs) noexcept
  {
    // Scale up to avoid losing precision
    auto result = static_cast<SaturationT>(value_) << FRACTIONAL_BITS;
    const auto rhs_value = static_cast<SaturationT>(rhs.value_);

    // Rounding, if signs are same, add 1/2 to the result, otherwise subtract 1/2.
    // This is to ensure that the result is rounded up for positive numbers and rounded down for negative numbers.
    const auto same_sign = sign_bit(result) == sign_bit(rhs_value);
    const auto half = rhs_value >> 1U;
    const SaturationT halfs[] = {-half, half}; // NOLINT(cppcoreguidelines-avoid-c-arrays)
    result += halfs[same_sign];

    // Divide
    result /= rhs_value;

    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator%=(const FixedPoint rhs) noexcept
  {
    const auto result = static_cast<SaturationT>(value_) % static_cast<SaturationT>(rhs.value_);
    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator++() noexcept
  {
    *this += FixedPoint(PRIVATE_CTOR, 1);
    return *this;
  }

  constexpr FixedPoint operator++(int) noexcept
  {
    const auto tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr FixedPoint& operator--() noexcept
  {
    *this -= FixedPoint(PRIVATE_CTOR, 1);
    return *this;
  }

  constexpr FixedPoint operator--(int) noexcept
  {
    const auto tmp = *this;
    --(*this);
    return tmp;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr FixedPoint operator+(FixedPoint lhs, const FixedPoint rhs) noexcept { return lhs += rhs; }
  friend constexpr FixedPoint operator-(FixedPoint lhs, const FixedPoint rhs) noexcept { return lhs -= rhs; }
  friend constexpr FixedPoint operator*(FixedPoint lhs, const FixedPoint rhs) noexcept { return lhs *= rhs; }
  friend constexpr FixedPoint operator/(FixedPoint lhs, const FixedPoint rhs) noexcept { return lhs /= rhs; }
  friend constexpr FixedPoint operator%(FixedPoint lhs, const FixedPoint rhs) noexcept { return lhs %= rhs; }

  /// Comparison operators
  /// @{
  friend constexpr bool operator==(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ == rhs.value_;
  }
  friend constexpr bool operator!=(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ != rhs.value_;
  }
  friend constexpr bool operator<(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ < rhs.value_;
  }
  friend constexpr bool operator>(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ > rhs.value_;
  }
  friend constexpr bool operator<=(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ <= rhs.value_;
  }
  friend constexpr bool operator>=(const FixedPoint lhs, const FixedPoint rhs) noexcept
  {
    return lhs.value_ >= rhs.value_;
  }
  /// @}

  friend std::ostream& operator<<(std::ostream& os, const FixedPoint value) noexcept
  {
    if constexpr (std::is_signed_v<T>)
    {
      os << "fp" << static_cast<std::int32_t>(INTEGER_BITS + 1U);
    }
    else
    {
      os << "ufp" << static_cast<std::int32_t>(INTEGER_BITS);
    }

    os << '.' << static_cast<std::int32_t>(FRACTIONAL_BITS);

    return os << "(" << static_cast<double>(value) << ")";
  }

  /// Math functions
  /// @{
  friend constexpr FixedPoint math::abs(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::floor(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::ceil(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::round(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::sqrt(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::sin(const FixedPoint value) noexcept;
  friend constexpr FixedPoint math::cos(const FixedPoint value) noexcept;
  /// @}

private:
  T value_;
};

using FixedPoint8 = FixedPoint<std::int16_t, 8, std::int32_t>;
using FixedPoint16 = FixedPoint<std::int32_t, 16, std::int64_t>;
using FixedPoint32 = FixedPoint<std::int64_t, 32, Int128>;

using FixedPoint8u = FixedPoint<std::uint16_t, 8, std::uint32_t>;
using FixedPoint16u = FixedPoint<std::uint32_t, 16, std::uint64_t>;
using FixedPoint32u = FixedPoint<std::uint64_t, 32, Int128u>;

} // namespace rtw::fixed_point

// std traits
namespace std
{

// NOLINTBEGIN(readability-identifier-naming)
template <typename T, std::int8_t FRAC_BITS, typename SaturationT>
struct numeric_limits<rtw::fixed_point::FixedPoint<T, FRAC_BITS, SaturationT>>
{
  using FixedPoint = rtw::fixed_point::FixedPoint<T, FRAC_BITS, SaturationT>;

  constexpr static bool is_specialized = true;
  constexpr static bool is_signed = std::is_signed_v<T>;
  constexpr static bool is_integer = false;
  constexpr static bool is_exact = true;
  constexpr static bool has_infinity = false;
  constexpr static bool has_quiet_NaN = false;
  constexpr static bool has_signaling_NaN = false;
  constexpr static float_denorm_style has_denorm = denorm_absent;
  constexpr static bool has_denorm_loss = false;
  constexpr static float_round_style round_style = round_to_nearest;
  constexpr static bool is_iec559 = false;
  constexpr static bool is_bounded = true;
  constexpr static bool is_modulo = false;
  constexpr static int digits = static_cast<int>(FixedPoint::FRACTIONAL_BITS);
  constexpr static int digits10 = static_cast<int>(FixedPoint::FRACTIONAL_BITS * rtw::math_constants::LOG10_2<double>);
  constexpr static int max_digits10 =
      static_cast<int>((FixedPoint::INTEGER_BITS + FixedPoint::FRACTIONAL_BITS) * rtw::math_constants::LOG10_2<double>);
  constexpr static int radix = 2;
  constexpr static int min_exponent = -static_cast<int>(FixedPoint::FRACTIONAL_BITS) + std::is_signed_v<T>;
  constexpr static int max_exponent = static_cast<int>(FixedPoint::INTEGER_BITS);
  constexpr static int min_exponent10 =
      -static_cast<int>(FixedPoint::FRACTIONAL_BITS) * std::log10(2) + std::is_signed_v<T>;
  constexpr static int max_exponent10 = static_cast<int>(FixedPoint::INTEGER_BITS * std::log10(2));
  constexpr static bool traps = numeric_limits<T>::traps;
  constexpr static bool tinyness_before = false;

  constexpr static FixedPoint lowest() noexcept { return FixedPoint::min(); }
  constexpr static FixedPoint min() noexcept { return FixedPoint::min(); }
  constexpr static FixedPoint max() noexcept { return FixedPoint::max(); }
  constexpr static FixedPoint epsilon() noexcept { return FixedPoint{1}; }
  constexpr static FixedPoint round_error() noexcept { return FixedPoint{1} >> 1; }
  constexpr static FixedPoint infinity() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint quiet_NaN() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint signaling_NaN() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint denorm_min() noexcept { return min(); }
};
// NOLINTEND(readability-identifier-naming)

} // namespace std
