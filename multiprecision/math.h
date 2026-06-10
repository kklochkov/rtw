#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace rtw::multiprecision::math
{

/// @brief Returns the sign bit of a signed integer.
/// @tparam T Integral type.
/// @param[in] value The integer value.
/// @return true if @p value is negative (sign bit set), false otherwise. Always false for unsigned types.
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr bool signbit(const T value) noexcept
{
  if constexpr (std::is_signed_v<T>)
  {
    return value < 0;
  }
  else
  {
    return false;
  }
}

constexpr std::int8_t sign(const bool value) noexcept = delete;

/// @brief Returns +1 or -1 based on the sign flag.
/// @tparam T Integral type.
/// @param[in] is_negative true for -1, false for +1.
/// @return +1 or -1.
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr std::int8_t sign(const T value) noexcept
{
  constexpr std::int8_t SIGNS[] = {1, -1};
  return SIGNS[signbit(value)]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

/// @brief Returns the absolute value of a signed integer (constexpr-friendly).
/// @tparam T Signed integral type.
/// @param[in] value The integer value.
/// @return |value|. For std::numeric_limits<T>::min(), saturates to max() (avoids signed overflow UB).
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_signed_v<T>>>
constexpr T abs(const T value) noexcept
{
  if (value == std::numeric_limits<T>::min())
  {
    return std::numeric_limits<T>::max();
  }
  return sign(value) * value;
}

/// @brief Counts the number of leading zero bits in an unsigned integer.
/// @tparam T Unsigned integral type.
/// @param[in] value The value to count leading zeros of.
/// @return Number of leading zero bits (0 to sizeof(T)*8).
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

/// @brief Counts leading zeros across a hi:lo pair (double-width integer).
/// @tparam HiT Unsigned type for the high half.
/// @tparam LoT Unsigned type for the low half.
/// @param[in] hi High half of the value.
/// @param[in] lo Low half of the value.
/// @param[in] hi_bits Number of bits in the high half.
/// @return Total number of leading zero bits across both halves.
template <typename HiT, typename LoT, typename = std::enable_if_t<std::is_integral_v<HiT> && std::is_unsigned_v<HiT>>>
constexpr std::int32_t count_leading_zero(const HiT hi, const LoT lo, const std::int32_t hi_bits) noexcept
{
  const auto hi_count = count_leading_zero(hi);
  return hi_count + (static_cast<std::int32_t>(hi_count == hi_bits) * count_leading_zero(lo));
}

/// @brief Rounds a floating-point value to the nearest integer (half away from zero).
/// @tparam F Floating-point type.
/// @param[in] value The value to round.
/// @return The rounded value (still as floating-point type).
template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F round_to_nearest_integer(const F value) noexcept
{
  return value < F{0} ? value - F{0.5} : value + F{0.5};
}

/// @brief Computes the floating-point remainder (constexpr-friendly fmod).
/// @tparam F Floating-point type.
/// @param[in] dividend The dividend.
/// @param[in] divisor The divisor.
/// @pre divisor must not be zero.
/// @return The remainder of dividend/divisor, with the same sign as dividend.
template <typename F, typename = std::enable_if_t<std::is_floating_point_v<F>>>
constexpr F fmod(const F dividend, const F divisor) noexcept
{
  assert(divisor != F{0} && "fmod: division by zero");
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
