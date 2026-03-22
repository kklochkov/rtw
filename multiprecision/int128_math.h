#pragma once

#include "multiprecision/int128.h"

namespace rtw::multiprecision::math
{

template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr std::int32_t count_leading_zero(const Int<T> value)
{
  return count_leading_zero(value.hi(), value.lo(), Int<T>::HI_BITS);
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool signbit(const Int<T> value) noexcept
{
  return signbit(value.hi());
}

template <typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
constexpr Int<T> abs(const Int<T> value) noexcept
{
  return sign(value < Int<T>{0}) * value;
}

} // namespace rtw::multiprecision::math
