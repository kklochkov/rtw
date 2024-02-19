#pragma once

#include "fixed_point/operations.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace rtw::fixed_point {

/// The overflow policy.
/// - Saturate: The result is saturated to the minimum or maximum value.
/// - Wrap: The result wraps around.
enum class OverflowPolicy : std::uint8_t
{
  Saturate,
  Wrap
};

/// Fixed-point number representation using QM.N format (ARM notation).
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
/// @tparam FractionalBits The number of fractional bits.
/// @tparam SaturationT The type used for saturation.
/// @tparam Policy The overflow policy. It can be Saturate or Wrap.
/// TODO: Add support for Int128 and UInt128.
/// TODO: Multiplication should return a greater precision fixed-point type,
/// i.e. fp8_t * fp8_t = fp16_t, fp16_t * fp16_t = fp32_t (this requires Int128/UInt128).
template <typename T, std::uint8_t FractionalBits, typename SaturationT, OverflowPolicy Policy>
class FixedPoint
{
  struct private_ctor_t
  {
  };
  constexpr static private_ctor_t private_ctor = {};
  FixedPoint(private_ctor_t, const T value) : value_(value) {}

  enum class ArithmeticType : std::uint8_t
  {
    Integral,
    FloatingPoint,
  };

public:
  using type = T;
  using saturation_type = SaturationT;

  constexpr static std::uint8_t BITS = sizeof(T) * 8U;
  constexpr static std::uint8_t SATURATION_BITS = sizeof(SaturationT) * 8U;
  constexpr static std::uint8_t SIGN_BIT = std::is_signed_v<T> ? 1U : 0U;
  constexpr static std::uint8_t FRACTIONAL_BITS = FractionalBits;
  constexpr static std::uint8_t INTEGER_BITS = BITS - FRACTIONAL_BITS - SIGN_BIT;
  constexpr static T ONE = 1UL << FRACTIONAL_BITS;
  constexpr static double RESOLUTION = 1.0 / static_cast<double>(ONE);
  constexpr static T MAX_INTEGER = (SaturationT(1) << (BITS - SIGN_BIT)) - 1U;
  constexpr static T MIN_INTEGER = std::is_signed_v<T> ? -MAX_INTEGER - 1U : 0U;
  constexpr static T MAX = (1U << INTEGER_BITS) - (1U >> FRACTIONAL_BITS) - 1U;
  constexpr static T MIN = std::is_signed_v<T> ? -MAX - 1U : 0U;

  static_assert(FractionalBits < BITS, "The number of fractional bits must be less than the total bits");
  static_assert(std::is_integral_v<T>, "The underlying type must be an integral type");
  static_assert(std::is_signed_v<SaturationT> == std::is_signed_v<T>,
                "The saturation type must have the same sign as the underlying type");

  constexpr FixedPoint() noexcept = default;

  template <typename U, std::enable_if_t<std::is_integral_v<U>, ArithmeticType> = ArithmeticType::Integral>
  explicit FixedPoint(const U value) noexcept
  {
    if constexpr (Policy == OverflowPolicy::Saturate)
    {
      auto result = static_cast<SaturationT>(value) << FRACTIONAL_BITS;
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
    }
    else if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ = static_cast<T>(value) << FRACTIONAL_BITS;
    }
    else
    {
      static_assert(sizeof(T) == 0, "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
  }

  template <typename U, std::enable_if_t<std::is_floating_point_v<U>, ArithmeticType> = ArithmeticType::FloatingPoint>
  explicit FixedPoint(const U value) noexcept
  {
    // std::lround is constexpr since C++20
    if constexpr (Policy == OverflowPolicy::Saturate)
    {
      auto result = static_cast<SaturationT>(std::lround(value * ONE));
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
    }
    else if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ = static_cast<T>(std::lround(value * ONE));
    }
    else
    {
      static_assert(sizeof(T) == 0, "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr static FixedPoint min() noexcept { return FixedPoint(private_ctor, MIN_INTEGER); }
  constexpr static FixedPoint max() noexcept { return FixedPoint(private_ctor, MAX_INTEGER); }

  template <typename U, std::enable_if_t<std::is_integral_v<U>, ArithmeticType> = ArithmeticType::Integral>
  constexpr explicit operator U() const noexcept
  {
    return static_cast<U>(value_ >> FRACTIONAL_BITS);
  }

  template <typename U, std::enable_if_t<std::is_floating_point_v<U>, ArithmeticType> = ArithmeticType::FloatingPoint>
  constexpr explicit operator U() const noexcept
  {
    return static_cast<U>(value_) * static_cast<U>(RESOLUTION);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_signed_v<U>>>
  constexpr FixedPoint operator-() const noexcept
  {
    return FixedPoint(private_ctor, -value_);
  }

  constexpr FixedPoint& operator+=(const FixedPoint rhs) noexcept
  {
    if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ += rhs.value_;
      return *this;
    }
    else if constexpr (Policy == OverflowPolicy::Saturate)
    {
      auto result = static_cast<SaturationT>(value_) + static_cast<SaturationT>(rhs.value_);
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
      return *this;
    }
    else
    {
      static_assert(sizeof(T) == 0, "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr FixedPoint& operator-=(const FixedPoint rhs) noexcept
  {
    if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ -= rhs.value_;
      return *this;
    }
    else if constexpr (Policy == OverflowPolicy::Saturate)
    {
      auto result = static_cast<SaturationT>(value_) - static_cast<SaturationT>(rhs.value_);
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
      return *this;
    }
    else
    {
      static_assert(sizeof(T) == 0, "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr FixedPoint& operator*=(const FixedPoint rhs) noexcept
  {
    auto result = static_cast<SaturationT>(value_) * static_cast<SaturationT>(rhs.value_);

    // Rounding, if the result is negative, add 1/2 to the result
    result += (1U << (FRACTIONAL_BITS - 1));

    // Scale back to the original number of fractional bits
    result >>= FRACTIONAL_BITS;

    if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ = static_cast<T>(result);
      return *this;
    }
    else if constexpr (Policy == OverflowPolicy::Saturate)
    {
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
      return *this;
    }
    else
    {
      static_assert(sizeof(T) == 0, "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
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
    const SaturationT halfs[] = {-half, half};
    result += halfs[same_sign];

    // Divide
    result /= rhs_value;

    if constexpr (Policy == OverflowPolicy::Wrap)
    {
      value_ = static_cast<T>(result);
      return *this;
    }
    else if constexpr (Policy == OverflowPolicy::Saturate)
    {
      result = std::clamp(result, static_cast<SaturationT>(MIN_INTEGER), static_cast<SaturationT>(MAX_INTEGER));
      value_ = static_cast<T>(result);
      return *this;
    }
    else
    {
      static_assert(!sizeof(T*), "Unknown overflow policy"); // workaround before CWG2518/P2593R1
    }
  }

  constexpr bool operator==(const FixedPoint rhs) const noexcept { return value_ == rhs.value_; }
  constexpr bool operator!=(const FixedPoint rhs) const noexcept { return value_ != rhs.value_; }
  constexpr bool operator<(const FixedPoint rhs) const noexcept { return value_ < rhs.value_; }
  constexpr bool operator>(const FixedPoint rhs) const noexcept { return value_ > rhs.value_; }
  constexpr bool operator<=(const FixedPoint rhs) const noexcept { return value_ <= rhs.value_; }
  constexpr bool operator>=(const FixedPoint rhs) const noexcept { return value_ >= rhs.value_; }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator==(const U rhs) const noexcept
  {
    return *this == FixedPoint(rhs);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator!=(const U rhs) const noexcept
  {
    return *this != FixedPoint(rhs);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator<(const U rhs) const noexcept
  {
    return *this < FixedPoint(rhs);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator>(const U rhs) const noexcept
  {
    return *this > FixedPoint(rhs);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator<=(const U rhs) const noexcept
  {
    return *this <= FixedPoint(rhs);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr bool operator>=(const U rhs) const noexcept
  {
    return *this >= FixedPoint(rhs);
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline FixedPoint operator+(FixedPoint lhs, FixedPoint rhs) noexcept { return lhs += rhs; }
  friend constexpr inline FixedPoint operator-(FixedPoint lhs, FixedPoint rhs) noexcept { return lhs -= rhs; }
  friend constexpr inline FixedPoint operator*(FixedPoint lhs, FixedPoint rhs) noexcept { return lhs *= rhs; }
  friend constexpr inline FixedPoint operator/(FixedPoint lhs, FixedPoint rhs) noexcept { return lhs /= rhs; }

  /// Comparison operators
  /// @{
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator==(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) == rhs;
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator!=(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) != rhs;
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator<(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) < rhs;
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator>(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) > rhs;
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator<=(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) <= rhs;
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  friend constexpr inline bool operator>=(const U lhs, const FixedPoint rhs) noexcept
  {
    return FixedPoint(lhs) >= rhs;
  }
  /// @}

  friend inline std::ostream& operator<<(std::ostream& os, const FixedPoint value) noexcept
  {
    if constexpr (std::is_signed_v<T>)
    {
      os << "fp";
    }
    else
    {
      os << "ufp";
    }

    os << static_cast<std::int32_t>(FRACTIONAL_BITS);

    if constexpr (Policy == OverflowPolicy::Saturate)
    {
      os << "s";
    }
    else
    {
      os << "w";
    }

    return os << "(" << static_cast<double>(value) << ")";
  }
  /// @}
private:
  T value_{};
};

using fp8_t = FixedPoint<std::int16_t, 8, std::int32_t, OverflowPolicy::Saturate>;
using fp16_t = FixedPoint<std::int32_t, 16, std::int64_t, OverflowPolicy::Saturate>;
// using fp32_t = FixedPoint<std::int64_t, 32, Int128>; // TODO: Implement Int128

using ufp8_t = FixedPoint<std::uint16_t, 8, std::uint32_t, OverflowPolicy::Saturate>;
using ufp16_t = FixedPoint<std::uint32_t, 16, std::uint64_t, OverflowPolicy::Saturate>;
// using ufp32_t = FixedPoint<std::uint64_t, 32, UInt128>; // TODO: Implement UInt128

} // namespace rtw::fixed_point
