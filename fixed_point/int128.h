#pragma once

#include "fixed_point/operations.h"

#include <fmt/core.h>

#include <cstdint>

namespace rtw::fixed_point {

template <typename T>
class Int
{
public:
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  using HiType = T;
  using LoType = std::conditional_t<std::is_signed_v<T>, std::make_unsigned_t<T>, T>;

  constexpr Int() noexcept = default;
  constexpr Int(const HiType hi, const LoType lo) noexcept : hi_{hi}, lo_{lo} {}

  template <typename U>
  constexpr Int(const U lo) noexcept : hi_{static_cast<HiType>(-sign_bit(lo))}, lo_{static_cast<LoType>(lo)}
  {
  }

  constexpr HiType hi() const noexcept { return hi_; }
  constexpr LoType lo() const noexcept { return lo_; }

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
    const auto lhs_lo = static_cast<HiType>(lo_);
    const auto rhs_lo = static_cast<HiType>(rhs.lo_);
    const auto result = mul(lhs_lo, rhs_lo);
    hi_ = hi_ * rhs_lo + lhs_lo * rhs.hi_ + result.hi_;
    lo_ = result.lo_;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Int operator+(Int lhs, const Int rhs) noexcept { return lhs += rhs; }
  friend constexpr Int operator-(Int lhs, const Int rhs) noexcept { return lhs -= rhs; }
  friend constexpr Int operator*(Int lhs, const Int rhs) noexcept { return lhs *= rhs; }
  /// @}

private:
  constexpr static Int mul(const LoType a, const LoType b) noexcept
  {
    constexpr LoType HALF_WORD_SIZE = sizeof(LoType) * LoType{4};
    constexpr LoType LO_MASK = (LoType{1} << HALF_WORD_SIZE) - LoType{1};

    const LoType a_hi = a >> HALF_WORD_SIZE;
    const LoType a_lo = a & LO_MASK;
    const LoType b_hi = b >> HALF_WORD_SIZE;
    const LoType b_lo = b & LO_MASK;

    const LoType lo_lo_product = b_lo * a_lo;
    const LoType lo_lo_carry = lo_lo_product >> HALF_WORD_SIZE;

    const LoType lo_hi_product = b_lo * a_hi + lo_lo_carry;
    const LoType lo_hi_result = lo_hi_product & LO_MASK;
    const LoType lo_hi_carry = static_cast<HiType>(lo_hi_product) >> HALF_WORD_SIZE;

    const LoType hi_lo_product = b_hi * a_lo + lo_hi_result;
    const LoType hi_lo_carry = static_cast<HiType>(hi_lo_product) >> HALF_WORD_SIZE;

    return Int{static_cast<HiType>(b_hi * a_hi + lo_hi_carry + hi_lo_carry),
               static_cast<LoType>(static_cast<HiType>(a * b))};
  }

private:
  HiType hi_{};
  LoType lo_{};
};

using Int128 = Int<std::int64_t>;
using UInt128 = Int<std::uint64_t>;

} // namespace rtw::fixed_point
