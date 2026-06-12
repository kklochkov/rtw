#pragma once

#include "constants/math_constants.h"
#include "multiprecision/int128_math.h"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <type_traits>

namespace rtw::multiprecision
{

struct RawValueConstructTag
{
  // clang-format off: https://github.com/llvm/llvm-project/issues/62067, https://github.com/llvm/llvm-project/issues/53960
  enum class Tag : std::uint8_t { TAG };
  // clang-format on
  constexpr explicit RawValueConstructTag(Tag /*tag*/) noexcept {}
};
constexpr inline RawValueConstructTag RAW_VALUE_CONSTRUCT{RawValueConstructTag::Tag::TAG};

/// Fixed-point number representation using QM.N format (ARM notation).
/// See https://en.wikipedia.org/wiki/Q_(number_format) for more information.
/// More information about fixed-point arithmetic can be found here:
/// https://en.m.wikipedia.org/wiki/Fixed-point_arithmetic
/// Ranges are:
/// - Signed: -2^(M-1) to 2^(M-1) - 2^(-N)
/// - Unsigned: 0 to 2^(M-1) - 2^(-N)
///
/// @note **Overflow strategy**: All arithmetic operations promote to a wider `SaturationT` type
/// before computation, then clamp (saturate) the result to [MIN_INTEGER, MAX_INTEGER] before storing.
/// This guarantees no signed integer overflow UB in any arithmetic path. The saturation type must be
/// at least twice the width of T (e.g., int64_t for int32_t T, Int128 for int64_t T).
///
/// @note **Unsigned subtraction**: For unsigned T, subtracting a larger value from a smaller one wraps
/// through the unsigned domain before saturation, which effectively yields MAX rather than 0.
/// This is inherent to unsigned semantics and documented as wrap-around behavior.
///
/// @note The `operator-()` (unary negation) saturates `-MIN` to `MAX` for signed types,
/// matching the standard library's behavior of avoiding signed overflow UB on the most negative value.
///
/// @tparam T The underlying type of the fixed-point number.
/// @tparam FRAC_BITS The number of fractional bits.
/// @tparam SaturationT The wider type used for intermediate arithmetic (must be at least 2*sizeof(T) wide).
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
class FixedPoint
{
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
  constexpr static T ONE = T{1} << FRACTIONAL_BITS;
  constexpr static T HALF = ONE >> 1U;
  constexpr static double RESOLUTION = 1.0L / static_cast<double>(ONE);
  constexpr static double ULP = RESOLUTION; // Unit in the last place or Unit of least precision (ULP)
  constexpr static T MAX_INTEGER = std::numeric_limits<T>::max();
  constexpr static T MIN_INTEGER = std::numeric_limits<T>::min();
  constexpr static T FRACTION_MASK = ONE - 1U;
  constexpr static T INTEGER_MASK = ~FRACTION_MASK;
  constexpr static T PI_INTEGER = math::round_to_nearest_integer(math_constants::PI<double> * ONE);
  constexpr static T PI_2_INTEGER = math::round_to_nearest_integer(math_constants::PI_2<double> * ONE);
  constexpr static T PI_4_INTEGER = math::round_to_nearest_integer(math_constants::PI_4<double> * ONE);
  constexpr static T TWO_PI_INTEGER = math::round_to_nearest_integer(math_constants::TWO_PI<double> * ONE);

  static_assert(FRACTIONAL_BITS < BITS, "The number of fractional bits must be less than the total bits");
  static_assert(std::is_integral_v<T>, "The underlying type must be an integral type");
  static_assert((std::is_signed_v<SaturationT> == std::is_signed_v<T>)
                    || (IS_BIG_INT_V<SaturationT> && (IS_SIGNED_BIG_INT_V<SaturationT> == std::is_signed_v<T>)),
                "The saturation type must have the same sign as the underlying type");

  constexpr FixedPoint() noexcept = default;

  constexpr FixedPoint(RawValueConstructTag /*tag*/, const T value) noexcept : value_{value} {}

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
    const auto result = static_cast<SaturationT>(math::round_to_nearest_integer(value * ONE));
    value_ = saturate_and_cast(result);
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  /// Converting constructor between fixed-point types of different scale and/or width.
  /// Rescales the raw value from @p OTHER_FRAC_BITS to @c FRAC_BITS: widening the fractional
  /// part is lossless, narrowing rounds half away from zero. The result is saturated to this
  /// type's representable range. The non-template copy constructor is preferred for same-type
  /// arguments, so this never shadows it.
  template <typename OtherT, std::uint8_t OTHER_FRAC_BITS, typename OtherSaturationT>
  constexpr explicit FixedPoint(const FixedPoint<OtherT, OTHER_FRAC_BITS, OtherSaturationT> other) noexcept
  {
    // Use the wider of the two saturation types as the intermediate so neither the source raw
    // value nor the rescaled result can be truncated before saturation.
    using WideT = std::conditional_t<(sizeof(SaturationT) >= sizeof(OtherSaturationT)), SaturationT, OtherSaturationT>;
    auto raw = static_cast<WideT>(other.raw_value());
    if constexpr (FRAC_BITS >= OTHER_FRAC_BITS)
    {
      // Multiply by a positive power of two rather than left-shifting `raw`, which may be negative
      // (left shift of a negative value is undefined behaviour and rejected in constant expressions).
      raw *= WideT{1} << static_cast<std::uint32_t>(FRAC_BITS - OTHER_FRAC_BITS);
    }
    else
    {
      constexpr std::uint32_t SHIFT = OTHER_FRAC_BITS - FRAC_BITS;
      const WideT divisor = WideT{1} << SHIFT;
      const WideT half = divisor / WideT{2};
      // Round half away from zero (matching operator/=) and divide rather than right-shift to
      // avoid implementation-defined behaviour for negative values.
      raw += math::signbit(raw) ? -half : half;
      raw /= divisor;
    }
    value_ = static_cast<T>(std::clamp(raw, static_cast<WideT>(MIN_INTEGER), static_cast<WideT>(MAX_INTEGER)));
  }

  constexpr static FixedPoint min() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, MIN_INTEGER); }
  constexpr static FixedPoint max() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, MAX_INTEGER); }

  /// Constants
  /// @{
  constexpr static FixedPoint pi() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, PI_INTEGER); }
  constexpr static FixedPoint pi_2() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, PI_2_INTEGER); }
  constexpr static FixedPoint pi_4() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, PI_4_INTEGER); }
  constexpr static FixedPoint two_pi() noexcept { return FixedPoint(RAW_VALUE_CONSTRUCT, TWO_PI_INTEGER); }
  constexpr static FixedPoint tau() noexcept { return two_pi(); }
  /// @}

  constexpr T raw_value() const noexcept { return value_; }

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
    // Saturate: -MIN overflows, clamp to MAX instead.
    if (value_ == MIN_INTEGER)
    {
      return FixedPoint(RAW_VALUE_CONSTRUCT, MAX_INTEGER);
    }
    return FixedPoint(RAW_VALUE_CONSTRUCT, -value_);
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
    // Scale up to avoid losing precision
    auto result = static_cast<SaturationT>(value_) * static_cast<SaturationT>(rhs.value_);

    // Rounding to the nearest integer by adding 1/2 to the fractional part.
    // This rounding works symmetrically for positive and negative numbers.
    result += HALF;

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

    assert(rhs_value != 0);

    // Rounding to the nearest.
    // If signs are same, add rhs_value/2 to the result, otherwise subtract rhs_value/2.
    // This is to ensure that the result is rounded up for positive numbers and rounded down for negative numbers.
    const auto same_sign = math::signbit(result) == math::signbit(rhs_value);
    // Use division instead of right-shift to avoid implementation-defined behavior for negative values.
    const auto half = rhs_value / SaturationT{2};
    const SaturationT halfs[] = {-half, half}; // NOLINT(cppcoreguidelines-avoid-c-arrays)
    result += halfs[same_sign];

    // Divide
    result /= rhs_value;

    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator%=(const FixedPoint rhs) noexcept
  {
    assert(rhs.value_ != 0 && "Modulo by zero");
    const auto result = static_cast<SaturationT>(value_) % static_cast<SaturationT>(rhs.value_);
    value_ = saturate_and_cast(result);
    return *this;
  }

  constexpr FixedPoint& operator++() noexcept
  {
    *this += FixedPoint(RAW_VALUE_CONSTRUCT, 1);
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
    *this -= FixedPoint(RAW_VALUE_CONSTRUCT, 1);
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

private:
  T value_;
};

using FixedPoint8 = FixedPoint<std::int16_t, 8, std::int32_t>;
using FixedPoint16 = FixedPoint<std::int32_t, 16, std::int64_t>;
using FixedPoint32 = FixedPoint<std::int64_t, 32, Int128>;

using FixedPoint8U = FixedPoint<std::uint16_t, 8, std::uint32_t>;
using FixedPoint16U = FixedPoint<std::uint32_t, 16, std::uint64_t>;
using FixedPoint32U = FixedPoint<std::uint64_t, 32, Int128U>;

// ----------------------------------------------------------------------------

template <typename T>
struct IsFixedPoint : std::false_type
{};

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
struct IsFixedPoint<FixedPoint<T, FRAC_BITS, SaturationT>> : std::true_type
{};

template <typename T>
constexpr inline bool IS_FIXED_POINT_V = IsFixedPoint<T>::value;

template <typename T>
struct IsFixedPointSigned : std::false_type
{};

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
struct IsFixedPointSigned<FixedPoint<T, FRAC_BITS, SaturationT>> : std::bool_constant<std::is_signed_v<T>>
{};

template <typename T>
struct IsComplex : std::false_type
{};

template <typename T>
struct IsComplex<std::complex<T>> : std::true_type
{};

template <typename T>
constexpr inline bool IS_COMPLEX_V = IsComplex<T>::value;

template <typename T>
constexpr inline bool IS_SIGNED_FIXED_POINT_V = IsFixedPointSigned<T>::value;

template <typename T>
constexpr inline bool IS_ARITHMETIC_V =
    std::is_arithmetic_v<T> || IS_FIXED_POINT_V<T> || IS_BIG_INT_V<T> || IS_COMPLEX_V<T>;

} // namespace rtw::multiprecision

// std traits
namespace std
{

// NOLINTBEGIN(readability-identifier-naming)
template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
struct numeric_limits<rtw::multiprecision::FixedPoint<T, FRAC_BITS, SaturationT>>
{
  using FixedPoint = rtw::multiprecision::FixedPoint<T, FRAC_BITS, SaturationT>;

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
  constexpr static std::int32_t digits =
      static_cast<std::int32_t>(FixedPoint::INTEGER_BITS + FixedPoint::FRACTIONAL_BITS);
  constexpr static std::int32_t digits10 = static_cast<std::int32_t>(
      (FixedPoint::INTEGER_BITS + FixedPoint::FRACTIONAL_BITS) * rtw::math_constants::LOG10_2<double>);
  constexpr static std::int32_t max_digits10 = static_cast<std::int32_t>(
      (FixedPoint::INTEGER_BITS + FixedPoint::FRACTIONAL_BITS) * rtw::math_constants::LOG10_2<double>);
  constexpr static std::int32_t radix = 2;
  constexpr static std::int32_t min_exponent =
      -static_cast<std::int32_t>(FixedPoint::FRACTIONAL_BITS) + std::is_signed_v<T>;
  constexpr static std::int32_t max_exponent = static_cast<std::int32_t>(FixedPoint::INTEGER_BITS);
  constexpr static std::int32_t min_exponent10 =
      static_cast<std::int32_t>(-static_cast<std::int32_t>(FixedPoint::FRACTIONAL_BITS)
                                * rtw::math_constants::LOG10_2<double>)
      + std::is_signed_v<T>;
  constexpr static std::int32_t max_exponent10 =
      static_cast<std::int32_t>(FixedPoint::INTEGER_BITS * rtw::math_constants::LOG10_2<double>);
  constexpr static bool traps = numeric_limits<T>::traps;
  constexpr static bool tinyness_before = false;

  constexpr static FixedPoint lowest() noexcept { return FixedPoint::min(); }
  constexpr static FixedPoint min() noexcept { return epsilon(); }
  constexpr static FixedPoint max() noexcept { return FixedPoint::max(); }
  constexpr static FixedPoint epsilon() noexcept { return FixedPoint(rtw::multiprecision::RAW_VALUE_CONSTRUCT, 1); }
  constexpr static FixedPoint round_error() noexcept
  {
    return FixedPoint(rtw::multiprecision::RAW_VALUE_CONSTRUCT, FixedPoint::HALF);
  }
  constexpr static FixedPoint infinity() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint quiet_NaN() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint signaling_NaN() noexcept { return FixedPoint{0}; }
  constexpr static FixedPoint denorm_min() noexcept { return epsilon(); }
};
// NOLINTEND(readability-identifier-naming)

} // namespace std
