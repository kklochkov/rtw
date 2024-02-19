#pragma once

#include <cstdint>
#include <type_traits>

namespace rtw::fixed_point
{

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

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
constexpr inline std::int32_t count_leading_zero(const T value)
{
  constexpr std::uint32_t WORD_SIZE = sizeof(T) * 8U;

  std::int32_t count = 0U;
  for (std::uint32_t i = 0U; i < WORD_SIZE; ++i)
  {
    if ((value >> (WORD_SIZE - 1U - i)) & 1U)
    {
      break;
    }
    ++count;
  }

  return count;
}

} // namespace rtw::fixed_point
