#pragma once

#include "math/ostream.h"
#include "sw_renderer/color.h"
#include "sw_renderer/tex_coord.h"

#include <iostream>
#include <ostream>

namespace rtw::sw_renderer
{

/// Formats a texture coordinate as "TexCoord(u, v)".
/// @param[in,out] os The output stream.
/// @param[in] coord The texture coordinate to format.
/// @return Reference to the output stream.
template <typename T>
std::ostream& operator<<(std::ostream& os, const TexCoord<T>& coord)
{
  os << "TexCoord";
  return math::details::operator<< <math::details::FormatType::VECTOR>(os, coord.as_matrix());
}

/// Formats a Color as "Color(rgba: 0xRRGGBBAA, r: R, g: G, b: B, a: A, rf: R, gf: G, bf: B, af: A)".
/// @param[in,out] os The output stream.
/// @param[in] color The color to format.
/// @return Reference to the output stream.
inline std::ostream& operator<<(std::ostream& os, const Color color)
{
  const auto temp_format = os.flags();
  os << "Color(";
  os << std::hex << std::uppercase;
  os << "rgba: 0x" << color.rgba;
  os << std::dec;
  os << ", r: " << static_cast<std::int32_t>(color.r());
  os << ", g: " << static_cast<std::int32_t>(color.g());
  os << ", b: " << static_cast<std::int32_t>(color.b());
  os << ", a: " << static_cast<std::int32_t>(color.a());
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
