#pragma once

#include "math/format.h"
#include "sw_renderer/color.h"
#include "sw_renderer/tex_coord.h"

#include <fmt/ostream.h>

#include <iostream>
#include <ostream>

namespace rtw::sw_renderer
{

template <typename T>
std::ostream& operator<<(std::ostream& os, const TexCoord<T>& coord) noexcept
{
  os << "TexCoord";
  return math::details::operator<< <math::details::FormatType::VECTOR>(os, coord);
}

inline std::ostream& operator<<(std::ostream& os, const Color color) noexcept
{
  const auto temp_format = os.flags();
  os << "Color(";
  os << std::hex << std::uppercase;
  os << "rgba: 0x" << color.rgba;
  os << std::dec;
  os << ", r: " << static_cast<int>(color.r());
  os << ", g: " << static_cast<int>(color.g());
  os << ", b: " << static_cast<int>(color.b());
  os << ", a: " << static_cast<int>(color.a());
  os << std::fixed << std::setprecision(4);
  os << ", rf: " << color.rf();
  os << ", gf: " << color.gf();
  os << ", bf: " << color.bf();
  os << ", af: " << color.af();
  os << ')';
  os.flags(temp_format);
  return os;
}

} // namespace rtw::sw_renderer

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::sw_renderer::TexCoord<T>> : ostream_formatter
{};

template <>
struct formatter<rtw::sw_renderer::Color> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
