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
constexpr bool sign_bit(const T value) noexcept
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

constexpr std::int8_t sign(const bool is_negative) noexcept
{
  constexpr std::int8_t SIGNS[] = {1, -1};
  return SIGNS[is_negative]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
constexpr std::int32_t count_leading_zero(const T value) noexcept
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

template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F round_to_nearest_integer(const F value) noexcept
{
  return value < F{0} ? value - F{0.5} : value + F{0.5};
}

template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F fmod(const F dividend, const F divisor) noexcept
{
  return dividend - (divisor * static_cast<F>(static_cast<std::int64_t>(dividend / divisor)));
}

} // namespace rtw::fixed_point
