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

  using HiType = T;
  using LoType = std::conditional_t<std::is_signed_v<T>, std::make_unsigned_t<T>, T>;

  constexpr static std::uint32_t HI_BITS = sizeof(HiType) * 8U;
  constexpr static std::uint32_t LO_BITS = sizeof(LoType) * 8U;
  constexpr static std::uint32_t BITS = HI_BITS + LO_BITS;
  constexpr static HiType MAX_HI =
      std::is_signed_v<HiType> ? ((HiType(1) << (HI_BITS - 1U)) - 1U) : (HiType(1) << HI_BITS);
  constexpr static HiType MIN_HI = std::is_signed_v<HiType> ? -MAX_HI - 1U : 0U;
  constexpr static LoType MAX_LO = LoType(1) << LO_BITS;
  constexpr static LoType MIN_LO = 0U;

  constexpr Int() noexcept = default;
  constexpr Int(const HiType hi, const LoType lo) noexcept : hi_{hi}, lo_{lo} {}

  // NOLINTBEGIN (google-explicit-constructor)
  template <typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
  constexpr Int(const U value) noexcept : hi_{static_cast<HiType>(-sign_bit(value))}, lo_{static_cast<LoType>(value)}
  {
  }

  template <typename U, typename = std::enable_if_t<std::is_integral_v<U>>>
  constexpr Int(const Int<U> other) noexcept
      : hi_{static_cast<HiType>(other.hi())}, lo_{static_cast<LoType>(other.lo())}
  {
  }
  // NOLINTEND (google-explicit-constructor)

  constexpr HiType hi() const noexcept { return hi_; }
  constexpr LoType lo() const noexcept { return lo_; }

  constexpr Int min() const noexcept { return Int{MIN_HI, MIN_LO}; }
  constexpr Int max() const noexcept { return Int{MAX_HI, MAX_LO}; }

  constexpr Int& operator<<=(const std::uint32_t shift) noexcept
  {
    if (shift >= LO_BITS)
    {
      hi_ = lo_ << (shift - LO_BITS);
      lo_ = LoType{0U};
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
      hi_ = HiType{0U};
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
    hi_ += rhs.hi_ + static_cast<HiType>(lo_ < old_lo);
    return *this;
  }

  constexpr Int& operator-=(const Int rhs) noexcept
  {
    const auto old_lo = lo_;
    lo_ -= rhs.lo_;
    hi_ -= rhs.hi_ + static_cast<HiType>(lo_ > old_lo);
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
    return Int{static_cast<HiType>(~lhs.hi_ + static_cast<HiType>(lhs.lo_ == 0U)), static_cast<LoType>(~lhs.lo_ + 1U)};
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
  constexpr static Int mul(const LoType a, const LoType b) noexcept
  {
    constexpr LoType HALF_WORD_SIZE = sizeof(LoType) * 4U;
    constexpr LoType LO_MASK = (LoType{1U} << HALF_WORD_SIZE) - 1U;

    const LoType a_hi = a >> HALF_WORD_SIZE;
    const LoType a_lo = a & LO_MASK;
    const LoType b_hi = b >> HALF_WORD_SIZE;
    const LoType b_lo = b & LO_MASK;

    const LoType lo_lo_product = b_lo * a_lo;
    const LoType lo_lo_carry = lo_lo_product >> HALF_WORD_SIZE;

    const LoType lo_hi_product = b_lo * a_hi + lo_lo_carry;
    const LoType lo_hi_result = lo_hi_product & LO_MASK;
    const LoType lo_hi_carry = lo_hi_product >> HALF_WORD_SIZE;

    const LoType hi_lo_product = b_hi * a_lo + lo_hi_result;
    const LoType hi_lo_carry = hi_lo_product >> HALF_WORD_SIZE;

    return Int{static_cast<HiType>(b_hi * a_hi + lo_hi_carry + hi_lo_carry), static_cast<LoType>(a * b)};
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

    const auto shift = count_leading_zero(divisor) - count_leading_zero(dividend);
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
    if constexpr (std::is_signed_v<HiType>)
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
  HiType hi_{};
  LoType lo_{};
};

using Int128 = Int<std::int64_t>;
using Int128u = Int<std::uint64_t>;

// ----------------------------------------------------------------------------

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr inline std::int32_t count_leading_zero(const Int<T> value)
{
  const auto hi_count = count_leading_zero(value.hi());
  return hi_count + static_cast<std::uint32_t>(hi_count == Int<T>::HI_BITS) * count_leading_zero(value.lo());
}

} // namespace rtw::fixed_point
