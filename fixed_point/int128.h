#pragma once

#include "fixed_point/operations.h"

#include <cassert>
#include <cstdint>

namespace rtw::fixed_point
{

template <typename T>
class Int
{
public:
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  using hi_type = T;
  using lo_type = std::conditional_t<std::is_signed_v<T>, std::make_unsigned_t<T>, T>;

  constexpr static std::uint32_t HI_BITS = sizeof(hi_type) * 8U;
  constexpr static std::uint32_t LO_BITS = sizeof(lo_type) * 8U;
  constexpr static std::uint32_t BITS = HI_BITS + LO_BITS;
  constexpr static hi_type MAX_HI =
      std::is_signed_v<hi_type> ? ((hi_type{1U} << (HI_BITS - 1U)) - 1U) : (hi_type{1U} << HI_BITS);
  constexpr static hi_type MIN_HI = std::is_signed_v<hi_type> ? -MAX_HI - 1U : 0U;
  constexpr static lo_type MAX_LO = lo_type{1U} << LO_BITS;
  constexpr static lo_type MIN_LO = 0U;

  constexpr Int() noexcept = default;
  constexpr Int(const hi_type hi, const lo_type lo) noexcept : hi_{hi}, lo_{lo} {}

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  template <typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
  constexpr Int(const U value) noexcept : hi_{static_cast<hi_type>(-sign_bit(value))}, lo_{static_cast<lo_type>(value)}
  {
  }

  template <typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
  constexpr Int(const Int<U> other) noexcept
      : hi_{static_cast<hi_type>(other.hi())}, lo_{static_cast<lo_type>(other.lo())}
  {
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr hi_type hi() const noexcept { return hi_; }
  constexpr lo_type lo() const noexcept { return lo_; }

  constexpr Int min() const noexcept { return Int{MIN_HI, MIN_LO}; }
  constexpr Int max() const noexcept { return Int{MAX_HI, MAX_LO}; }

  constexpr explicit operator double() const noexcept
  {
    constexpr double POW_2_64 = 18'446'744'073'709'551'616.0;
    if constexpr (std::is_signed_v<hi_type>)
    {
      if ((hi_ < 0) && (hi_ != MIN_HI) && (lo_ != MIN_LO))
      {
        return -static_cast<double>(-*this);
      }

      return static_cast<double>(hi_) * POW_2_64 + static_cast<double>(lo_);
    }
    else
    {
      return static_cast<double>(hi_) * POW_2_64 + static_cast<double>(lo_);
    }
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
      hi_ = hi_type{0U};
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

  constexpr bool operator==(const Int rhs) const noexcept { return (hi_ == rhs.hi_) && (lo_ == rhs.lo_); }
  constexpr bool operator!=(const Int rhs) const noexcept { return !(*this == rhs); }
  constexpr bool operator<(const Int rhs) const noexcept
  {
    return (hi_ < rhs.hi_) || ((hi_ == rhs.hi_) && (lo_ < rhs.lo_));
  }
  constexpr bool operator>(const Int rhs) const noexcept { return rhs < *this; }
  constexpr bool operator<=(const Int rhs) const noexcept { return !(*this > rhs); }
  constexpr bool operator>=(const Int rhs) const noexcept { return !(*this < rhs); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Int operator+(Int lhs) noexcept { return lhs; }
  friend constexpr Int operator-(Int lhs) noexcept
  {
    return Int{static_cast<hi_type>(~lhs.hi_ + static_cast<hi_type>(lhs.lo_ == 0U)),
               static_cast<lo_type>(~lhs.lo_ + 1U)};
  }
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
  hi_type hi_{};
  lo_type lo_{};
};

using Int128 = Int<std::int64_t>;
using Int128u = Int<std::uint64_t>;

// ----------------------------------------------------------------------------

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr std::int32_t count_leading_zero(const Int<T> value)
{
  const auto hi_count = count_leading_zero(value.hi());
  return hi_count + static_cast<std::uint32_t>(hi_count == Int<T>::HI_BITS) * count_leading_zero(value.lo());
}

} // namespace rtw::fixed_point
