#pragma once

#include "stl/flags.h"
#include "stl/static_string.h"
#include "stl/string_view.h"

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
    os << values[(static_cast<typename Flags<T>::underlying_type>(flags) >> i) & 1U];
  }

  os << ')';
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const StringView view) noexcept
{
  return os.write(view.data(), static_cast<std::streamsize>(view.size()));
}

template <typename DerivedT>
inline std::ostream& operator<<(std::ostream& os, const GenericStaticString<DerivedT>& string) noexcept
{
  return os.write(string.data(), static_cast<std::streamsize>(string.size()));
}

} // namespace rtw::stl

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::stl::Flags<T>> : ostream_formatter
{};

template <>
struct formatter<rtw::stl::StringView> : ostream_formatter
{};

template <typename DerivedT>
struct formatter<rtw::stl::GenericStaticString<DerivedT>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
