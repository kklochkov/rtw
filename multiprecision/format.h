#pragma once

#include "multiprecision/fixed_point.h"
#include "multiprecision/int128.h"

#include <fmt/ostream.h>

#include <iomanip>
#include <ostream>

namespace rtw::multiprecision
{

template <typename T>
std::ostream& operator<<(std::ostream& os, const Int<T> value) noexcept
{
  const auto temp_format = os.flags();
  os << "0x" << std::hex << std::setw(16) << std::setfill('0') << value.hi() << std::hex << std::setw(16)
     << std::setfill('0') << value.lo();
  os.flags(temp_format);
  return os;
}

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
std::ostream& operator<<(std::ostream& os, const FixedPoint<T, FRAC_BITS, SaturationT> value) noexcept
{
  using FixedPoint = FixedPoint<T, FRAC_BITS, SaturationT>;

  const auto temp_format = os.flags();
  if constexpr (std::is_signed_v<T>)
  {
    os << "fp" << static_cast<std::int32_t>(FixedPoint::INTEGER_BITS + 1U);
  }
  else
  {
    os << "ufp" << static_cast<std::int32_t>(FixedPoint::INTEGER_BITS);
  }

  os << '.' << static_cast<std::int32_t>(FixedPoint::FRACTIONAL_BITS);
  os << '(' << static_cast<double>(value) << ')';
  os.flags(temp_format);
  return os;
}

} // namespace rtw::multiprecision

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::multiprecision::Int<T>> : ostream_formatter
{};

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
struct formatter<rtw::multiprecision::FixedPoint<T, FRAC_BITS, SaturationT>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
