#pragma once

#include "stl/flags.h"

#include <fmt/ostream.h>

#include <iostream>
#include <ostream>

namespace rtw::stl
{

template <typename T>
std::ostream& operator<<(std::ostream& os, const Flags<T>& flags) noexcept
{
  os << "Flags(";

  const auto bits_count = sizeof(T) * 8U;
  const char values[] = {'0', '1'};
  for (std::size_t i = bits_count; i--;)
  {
    os << values[(flags.flags_ >> i) & 1U];
  }

  os << ')';
  return os;
}

} // namespace rtw::stl

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::stl::Flags<T>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
