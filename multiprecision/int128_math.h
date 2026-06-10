#pragma once

#include "multiprecision/int128.h"

namespace rtw::multiprecision::math
{

/// @brief Counts leading zero bits in a double-width integer.
/// @tparam T Unsigned integral type for the high half.
/// @param[in] value The Int<T> value.
/// @return Number of leading zero bits (0 to 2*sizeof(T)*8).
template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr std::int32_t count_leading_zero(const Int<T> value)
{
  return count_leading_zero(value.hi(), value.lo(), Int<T>::HI_BITS);
}

/// @brief Returns the sign bit of a double-width integer.
/// @tparam T Integral type for the high half.
/// @param[in] value The Int<T> value.
/// @return true if @p value is negative, false otherwise.
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool signbit(const Int<T> value) noexcept
{
  return signbit(value.hi());
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr std::int8_t sign(const Int<T> value) noexcept
{
  return sign(value.hi());
}

/// @brief Returns the absolute value of a signed double-width integer.
/// @tparam T Signed integral type for the high half.
/// @param[in] value The Int<T> value.
/// @return |value|. For min() (cannot be negated), saturates to max().
template <typename T, typename = std::enable_if_t<std::is_signed_v<T>>>
constexpr Int<T> abs(const Int<T> value) noexcept
{
  if (value == Int<T>::min())
  {
    return Int<T>::max();
  }
  return sign(value) * value;
}

} // namespace rtw::multiprecision::math
