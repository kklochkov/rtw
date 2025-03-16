#pragma once

#include "constants/math_constants.h"
#include "fixed_point/operations.h"

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <limits>

namespace rtw::fixed_point
{

template <typename T>
class Int;

template <typename T>
std::ostream& operator<<(std::ostream& os, const Int<T> value) noexcept;

enum class ArithmeticType : std::uint8_t
{
  INTEGRAL,
  FLOATING_POINT,
};

template <typename T>
class Int
{
  template <typename F>
  constexpr static F get_pow_2_64() noexcept
  {
    return static_cast<F>(18'446'744'073'709'551'616.0);
  }

public:
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  using hi_type = T;
  using lo_type = std::make_unsigned_t<T>;

  constexpr static std::uint32_t HI_BITS = sizeof(hi_type) * 8U;
  constexpr static std::uint32_t LO_BITS = sizeof(lo_type) * 8U;
  constexpr static std::uint32_t BITS = HI_BITS + LO_BITS;
  constexpr static lo_type MAX_LO = ~lo_type{0U};
  constexpr static lo_type MIN_LO = 0U;
  constexpr static hi_type MAX_HI = std::is_signed_v<hi_type> ? (lo_type{1U} << (HI_BITS - 1U)) - 1U : MAX_LO;
  constexpr static hi_type MIN_HI = std::is_signed_v<hi_type> ? -MAX_HI - 1U : MIN_LO;

  constexpr Int() noexcept = default;
  constexpr Int(const hi_type hi, const lo_type lo) noexcept : hi_{hi}, lo_{lo} {}

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr Int(const I value) noexcept : hi_{static_cast<hi_type>(-sign_bit(value))}, lo_{static_cast<lo_type>(value)}
  {
  }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr Int(const F value) noexcept
  {
    constexpr auto POW_2_64 = get_pow_2_64<F>();
    if (value < 0)
    {
      if constexpr (std::is_signed_v<hi_type>)
      {
        lo_ = static_cast<lo_type>(fmod(-value, POW_2_64));
        hi_ = static_cast<hi_type>(-value / POW_2_64);

        lo_ = ~lo_ + 1U;
        hi_ = ~hi_ + static_cast<hi_type>(lo_ == 0U);
      }
      else
      {
        hi_ = hi_type{0U};
        lo_ = static_cast<lo_type>(value);
      }
    }
    else
    {
      lo_ = static_cast<lo_type>(fmod(value, POW_2_64));
      hi_ = static_cast<hi_type>(value / POW_2_64);
    }
  }

  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr Int(const Int<I> other) noexcept
      : hi_{static_cast<hi_type>(other.hi())}, lo_{static_cast<lo_type>(other.lo())}
  {
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr hi_type hi() const noexcept { return hi_; }
  constexpr lo_type lo() const noexcept { return lo_; }

  constexpr static Int min() noexcept { return Int{MIN_HI, MIN_LO}; }
  constexpr static Int max() noexcept { return Int{MAX_HI, MAX_LO}; }

  constexpr explicit operator bool() const noexcept { return hi_ || lo_; }

  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr explicit operator I() const noexcept
  {
    return static_cast<I>(lo_);
  }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr explicit operator F() const noexcept
  {
    if constexpr (std::is_signed_v<hi_type>)
    {
      if ((hi_ < 0) && (hi_ != MIN_HI) && (lo_ != MIN_LO))
      {
        return -static_cast<F>(-*this);
      }
    }

    constexpr auto POW_2_64 = get_pow_2_64<F>();
    return static_cast<F>(hi_) * POW_2_64 + static_cast<F>(lo_);
  }

  template <typename U = T, typename = std::enable_if_t<std::is_signed_v<U>>>
  constexpr Int operator-() const noexcept
  {
    return Int{-hi_, -lo_};
  }

  constexpr Int& operator<<=(const std::uint32_t shift) noexcept
  {
    if (shift >= LO_BITS)
    {
      hi_ = lo_ << (shift - LO_BITS);
      lo_ = lo_type{0U};
    }
    else if (shift > 0U)
    {
      hi_ = (hi_ << shift) | (lo_ >> (LO_BITS - shift));
      lo_ <<= shift;
    }
    return *this;
  }

  constexpr Int& operator>>=(const std::uint32_t shift) noexcept
  {
    if (shift >= LO_BITS)
    {
      lo_ = hi_ >> (shift - LO_BITS);
      if constexpr (std::is_unsigned_v<hi_type>)
      {
        hi_ = hi_type{0U};
      }
      else
      {
        hi_ >>= (LO_BITS - 1U); // Fill with the sign bit.
      }
    }
    else if (shift > 0U)
    {
      lo_ = (lo_ >> shift) | (hi_ << (LO_BITS - shift));
      hi_ >>= shift;
    }
    return *this;
  }

  constexpr Int& operator&=(const Int rhs) noexcept
  {
    hi_ &= rhs.hi_;
    lo_ &= rhs.lo_;
    return *this;
  }

  constexpr Int& operator|=(const Int rhs) noexcept
  {
    hi_ |= rhs.hi_;
    lo_ |= rhs.lo_;
    return *this;
  }

  constexpr Int& operator^=(const Int rhs) noexcept
  {
    hi_ ^= rhs.hi_;
    lo_ ^= rhs.lo_;
    return *this;
  }

  constexpr Int& operator+=(const Int rhs) noexcept
  {
    const auto old_lo = lo_;
    lo_ += rhs.lo_;
    hi_ += rhs.hi_ + static_cast<hi_type>(lo_ < old_lo);
    return *this;
  }

  constexpr Int& operator-=(const Int rhs) noexcept
  {
    const auto old_lo = lo_;
    lo_ -= rhs.lo_;
    hi_ -= rhs.hi_ + static_cast<hi_type>(lo_ > old_lo);
    return *this;
  }

  constexpr Int& operator*=(const Int rhs) noexcept
  {
    *this = mul(*this, rhs);
    return *this;
  }

  constexpr Int& operator/=(const Int rhs) noexcept
  {
    *this = div(*this, rhs).quotient;
    return *this;
  }

  constexpr Int& operator%=(const Int rhs) noexcept
  {
    *this = div(*this, rhs).reminder;
    return *this;
  }

  constexpr Int& operator++() noexcept
  {
    *this += Int{1};
    return *this;
  }

  constexpr Int operator++(int) noexcept
  {
    const auto result = *this;
    ++(*this);
    return result;
  }

  constexpr Int& operator--() noexcept
  {
    *this -= Int{1};
    return *this;
  }

  constexpr Int operator--(int) noexcept
  {
    const auto result = *this;
    --(*this);
    return result;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Int operator+(Int lhs, const Int rhs) noexcept { return lhs += rhs; }
  friend constexpr Int operator-(Int lhs, const Int rhs) noexcept { return lhs -= rhs; }
  friend constexpr Int operator*(Int lhs, const Int rhs) noexcept { return lhs *= rhs; }
  friend constexpr Int operator/(Int lhs, const Int rhs) noexcept { return lhs /= rhs; }
  friend constexpr Int operator%(Int lhs, const Int rhs) noexcept { return lhs %= rhs; }
  friend constexpr Int operator<<(Int lhs, const std::uint32_t shift) noexcept { return lhs <<= shift; }
  friend constexpr Int operator>>(Int lhs, const std::uint32_t shift) noexcept { return lhs >>= shift; }
  friend constexpr Int operator&(Int lhs, const Int rhs) noexcept { return lhs &= rhs; }
  friend constexpr Int operator|(Int lhs, const Int rhs) noexcept { return lhs |= rhs; }
  friend constexpr Int operator^(Int lhs, const Int rhs) noexcept { return lhs ^= rhs; }
  friend constexpr Int operator~(Int lhs) noexcept { return Int{~lhs.hi(), ~lhs.lo()}; }
  /// @}

  /// Comparison operators
  /// @{
  friend constexpr bool operator!(const Int lhs) noexcept { return (lhs.hi_ == 0U) && (lhs.lo_ == 0U); }
  friend constexpr bool operator==(const Int lhs, const Int rhs) noexcept
  {
    return (lhs.hi_ == rhs.hi_) && (lhs.lo_ == rhs.lo_);
  }
  friend constexpr bool operator!=(const Int lhs, const Int rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const Int lhs, const Int rhs) noexcept
  {
    return (lhs.hi_ < rhs.hi_) || ((lhs.hi_ == rhs.hi_) && (lhs.lo_ < rhs.lo_));
  }
  friend constexpr bool operator>(const Int lhs, const Int rhs) noexcept { return rhs < lhs; }
  friend constexpr bool operator<=(const Int lhs, const Int rhs) noexcept { return !(lhs > rhs); }
  friend constexpr bool operator>=(const Int lhs, const Int rhs) noexcept { return !(lhs < rhs); }

  friend constexpr Int operator+(Int lhs) noexcept { return lhs; }
  friend constexpr Int operator-(Int lhs) noexcept
  {
    return Int{static_cast<hi_type>(~lhs.hi_ + static_cast<hi_type>(lhs.lo_ == 0U)),
               static_cast<lo_type>(~lhs.lo_ + 1U)};
  }
  /// @}

  friend std::ostream& operator<< <T>(std::ostream& os, const Int value) noexcept;

private:
  /// Multiplication algorithm. Adjusted version of the algorithm from: Hacker's Delight, 2nd Edition
  /// @param a The first operand.
  /// @param b The second operand.
  /// @return The result of the multiplication.
  /// @{
  constexpr static Int mul(const lo_type a, const lo_type b) noexcept
  {
    constexpr lo_type HALF_WORD_SIZE = sizeof(lo_type) * 4U;
    constexpr lo_type LO_MASK = (lo_type{1U} << HALF_WORD_SIZE) - 1U;

    const lo_type a_hi = a >> HALF_WORD_SIZE;
    const lo_type a_lo = a & LO_MASK;
    const lo_type b_hi = b >> HALF_WORD_SIZE;
    const lo_type b_lo = b & LO_MASK;

    const lo_type lo_lo_product = b_lo * a_lo;
    const lo_type lo_lo_carry = lo_lo_product >> HALF_WORD_SIZE;

    const lo_type lo_hi_product = b_lo * a_hi + lo_lo_carry;
    const lo_type lo_hi_result = lo_hi_product & LO_MASK;
    const lo_type lo_hi_carry = lo_hi_product >> HALF_WORD_SIZE;

    const lo_type hi_lo_product = b_hi * a_lo + lo_hi_result;
    const lo_type hi_lo_carry = hi_lo_product >> HALF_WORD_SIZE;

    return Int{static_cast<hi_type>(b_hi * a_hi + lo_hi_carry + hi_lo_carry), static_cast<lo_type>(a * b)};
  }

  constexpr static Int mul(const Int lhs, const Int rhs) noexcept
  {
    auto result = mul(lhs.lo_, rhs.lo_);
    result.hi_ += lhs.hi_ * rhs.lo_ + lhs.lo_ * rhs.hi_;
    return result;
  }
  /// @}

  /// Division algorithm. Adjusted version of the algorithm from: Hacker's Delight, 2nd Edition
  /// @{
  template <typename U>
  struct DivResult
  {
    Int<U> quotient{};
    Int<U> reminder{};
  };

  /// Shift-and-subtract division algorithm. Adjusted version of the algorithm from: Hacker's Delight, 2nd Edition
  /// and the blog post: https://blog.segger.com/algorithms-for-division-part-2-classics
  /// @param dividend The dividend.
  /// @param divisor The divisor.
  /// @return The result of the division.
  template <typename U = T, typename = std::enable_if_t<std::is_unsigned_v<U>>>
  constexpr static DivResult<U> div_unsigned(Int<U> dividend, Int<U> divisor) noexcept
  {
    assert(divisor != Int<U>{0U} && "Division by zero");

    if (dividend < divisor)
    {
      return DivResult<U>{0U, dividend};
    }

    if (divisor == dividend)
    {
      return DivResult<U>{1U, 0U};
    }

    DivResult<U> result;
    result.reminder = dividend;

    const std::int32_t shift = count_leading_zero(divisor) - count_leading_zero(dividend);
    divisor <<= shift;

    for (std::int32_t i = 0; i <= shift; ++i)
    {
      result.quotient <<= 1U;
      if (result.reminder >= divisor)
      {
        result.reminder -= divisor;
        result.quotient |= 1U;
      }
      divisor >>= 1U;
    }

    return result;
  }

  /// Shift-and-subtract division algorithm. Adjusted version of the algorithm from: Hacker's Delight, 2nd Edition
  template <typename U = T, typename = std::enable_if_t<std::is_signed_v<U>>>
  constexpr static DivResult<U> div_signed(Int<U> dividend, Int<U> divisor) noexcept
  {
    using UInt = Int<std::make_unsigned_t<U>>;
    using Int = Int<U>;

    const auto dividend_sign = sign_bit(dividend.hi());
    const auto divisor_sign = sign_bit(divisor.hi());

    if (dividend_sign)
    {
      dividend = -dividend;
    }

    if (divisor_sign)
    {
      divisor = -divisor;
    }

    const auto dividend_unsigned = static_cast<UInt>(dividend);
    const auto divisor_unsigned = static_cast<UInt>(divisor);
    const auto div_result = div_unsigned(dividend_unsigned, divisor_unsigned);

    auto quotient = static_cast<Int>(div_result.quotient);
    if (dividend_sign != divisor_sign)
    {
      quotient = -quotient;
    }

    auto reminder = static_cast<Int>(div_result.reminder);
    if (dividend_sign)
    {
      reminder = -reminder;
    }

    return DivResult<U>{quotient, reminder};
  }

  constexpr static DivResult<T> div(Int dividend, Int divisor) noexcept
  {
    if constexpr (std::is_signed_v<hi_type>)
    {
      return div_signed(dividend, divisor);
    }
    else
    {
      return div_unsigned(dividend, divisor);
    }
  }
  /// @}

private:
  hi_type hi_;
  lo_type lo_;
};

using Int128 = Int<std::int64_t>;
using Int128U = Int<std::uint64_t>;

// ----------------------------------------------------------------------------

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr std::int32_t count_leading_zero(const Int<T> value)
{
  const auto hi_count = count_leading_zero(value.hi());
  return hi_count + static_cast<std::uint32_t>(hi_count == Int<T>::HI_BITS) * count_leading_zero(value.lo());
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool sign_bit(const Int<T> value) noexcept
{
  return sign_bit(value.hi());
}

template <typename T>
struct IsBigInt : std::false_type
{};

template <typename T>
struct IsBigInt<Int<T>> : std::true_type
{};

template <typename T>
constexpr bool IS_BIG_INT_V = IsBigInt<T>::value;

template <typename T>
struct IsSignedBigInt : std::false_type
{};

template <typename T>
struct IsSignedBigInt<Int<T>> : std::bool_constant<std::is_signed_v<T>>
{};

template <typename T>
constexpr bool IS_SIGNED_BIG_INT_V = IsSignedBigInt<T>::value;

} // namespace rtw::fixed_point

// std traits
namespace std
{
// NOLINTBEGIN(readability-identifier-naming)

template <typename T>
struct numeric_limits<rtw::fixed_point::Int<T>>
{
  using Int = rtw::fixed_point::Int<T>;

  constexpr static bool is_specialized = true;
  constexpr static bool is_signed = std::is_signed_v<T>;
  constexpr static bool is_integer = true;
  constexpr static bool is_exact = true;
  constexpr static bool has_infinity = false;
  constexpr static bool has_quiet_NaN = false;
  constexpr static bool has_signaling_NaN = false;
  constexpr static float_denorm_style has_denorm = denorm_absent;
  constexpr static bool has_denorm_loss = false;
  constexpr static float_round_style round_style = round_toward_zero;
  constexpr static bool is_iec559 = false;
  constexpr static bool is_bounded = true;
  constexpr static bool is_modulo = !std::is_signed_v<T>;
  constexpr static int digits = Int::BITS - std::is_signed_v<T>; // Excluding the sign bit
  constexpr static int digits10 = static_cast<int>(Int::BITS * rtw::math_constants::LOG10_2<double>);
  constexpr static int max_digits10 = 0;
  constexpr static int radix = 2;
  constexpr static int min_exponent = 0;
  constexpr static int min_exponent10 = 0;
  constexpr static int max_exponent = 0;
  constexpr static int max_exponent10 = 0;
  constexpr static bool traps = numeric_limits<T>::traps;
  constexpr static bool tinyness_before = false;

  constexpr static Int min() noexcept { return Int::min(); }
  constexpr static Int lowest() noexcept { return Int::min(); }
  constexpr static Int max() noexcept { return Int::max(); }
  constexpr static Int epsilon() noexcept { return 0; }
  constexpr static Int round_error() noexcept { return 0; }
  constexpr static Int infinity() noexcept { return 0; }
  constexpr static Int quiet_NaN() noexcept { return 0; }
  constexpr static Int signaling_NaN() noexcept { return 0; }
  constexpr static Int denorm_min() noexcept { return 0; }
};
// NOLINTEND(readability-identifier-naming)

} // namespace std
