#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace rtw::multiprecision::math
{

/// Sign bit of an integer.
/// @tparam T The type of the integer.
/// @param[in] value The integer value.
/// @return For signed integers, it returns the sign bit, otherwise it returns false.
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool signbit(const T value) noexcept
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
  // Instead of the loop, compiler (GCC/Clang) intrinsics can be used:
  // - `__builtin_clz(value)` for 32-bit unsigned integers
  // - `__builtin_clzll(value)` for 64-bit unsigned integers

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

template <typename HiT, typename LoT, typename = std::enable_if_t<std::is_integral_v<HiT> && std::is_unsigned_v<HiT>>>
constexpr std::int32_t count_leading_zero(const HiT hi, const LoT lo, const std::int32_t hi_bits) noexcept
{
  const auto hi_count = count_leading_zero(hi);
  return hi_count + (static_cast<std::int32_t>(hi_count == hi_bits) * count_leading_zero(lo));
}

template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F round_to_nearest_integer(const F value) noexcept
{
  return value < F{0} ? value - F{0.5} : value + F{0.5};
}

template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F fmod(const F dividend, const F divisor) noexcept
{
  const auto quotient = dividend / divisor;
  // For values within int64 range, use the cast method for truncation
  // For larger values, floating-point can't represent fractions anyway
  if ((quotient >= static_cast<F>(std::numeric_limits<std::int64_t>::min()))
      && (quotient <= static_cast<F>(std::numeric_limits<std::int64_t>::max())))
  {
    return dividend - (divisor * static_cast<F>(static_cast<std::int64_t>(quotient)));
  }
  return dividend - (divisor * quotient);
}

} // namespace rtw::multiprecision::math
