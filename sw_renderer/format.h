#pragma once

#include "math/format.h"
#include "sw_renderer/color.h"
#include "sw_renderer/flags.h"
#include "sw_renderer/tex_coord.h"

#include <fmt/ostream.h>

#include <ostream>

namespace rtw::sw_renderer
{

template <typename T>
std::ostream& operator<<(std::ostream& os, const TexCoord<T>& coord) noexcept
{
  os << "TexCoord";
  return math::details::operator<<(os, coord);
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
  os << ")\n";
  os.flags(temp_format);
  return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Flags<T>& flags) noexcept
{
  os << "Flags(";

  const auto bits_count = sizeof(T) * 8U;
  const char values[] = {'0', '1'};
  for (std::size_t i = 0U; i < bits_count; ++i)
  {
    os << values[(flags.flags_ >> i) & 1U];
  }

  os << ")\n";
  return os;
}

} // namespace rtw::sw_renderer

namespace fmt
{

template <typename T>
struct formatter<rtw::sw_renderer::TexCoord<T>> : ostream_formatter
{};

template <>
struct formatter<rtw::sw_renderer::Color> : ostream_formatter
{};

} // namespace fmt
