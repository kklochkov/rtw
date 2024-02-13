#pragma once

#include <type_traits>

namespace rtw::fixed_point {

/// Sign bit of an integer.
/// @tparam T The type of the integer.
/// @param value The integer value.
/// @return For signed integers, it returns the sign bit, otherwise it returns false.
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr inline bool sign_bit(const T value)
{
  if constexpr (std::is_signed_v<T>)
  {
    return (value >> (sizeof(T) * 8U - 1U)) & 1U;
  }
  else
  {
    return false;
  }
}

} // namespace rtw::fixed_point
