#pragma once

#include "math/interpolation.h"

#include <cstdint>
#include <ostream>
#include <type_traits>

namespace rtw::sw_renderer {

struct Color
{
  constexpr Color() = default;
  constexpr explicit Color(const std::uint32_t color) : rgba{color} {}

  template <typename T = std::uint8_t, std::enable_if_t<std::is_integral_v<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = 0xFF) : rgba(r << 24U | g << 16U | b << 8U | a)
  {
  }

  template <typename T = float, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = 1.0F)
      : Color{static_cast<std::uint8_t>(r * 255.0F), static_cast<std::uint8_t>(g * 255.0F),
              static_cast<std::uint8_t>(b * 255.0F), static_cast<std::uint8_t>(a * 255.0F)}
  {
  }

  constexpr void set_r(const std::uint8_t r) { rgba = (rgba & 0x00FFFFFF) | (r << 24U); }
  constexpr std::uint8_t r() const { return (rgba >> 24U) & 0xFF; }

  constexpr void set_rf(const float r) { set_r(static_cast<std::uint8_t>(r * 255.0F)); }
  constexpr float rf() const { return static_cast<float>(r()) / 255.0F; }

  constexpr void set_g(const std::uint8_t g) { rgba = (rgba & 0xFF00FFFF) | (g << 16U); }
  constexpr std::uint8_t g() const { return (rgba >> 16U) & 0xFF; }

  constexpr void set_gf(const float g) { set_g(static_cast<std::uint8_t>(g * 255.0F)); }
  constexpr float gf() const { return static_cast<float>(g()) / 255.0F; }

  constexpr void set_b(const std::uint8_t b) { rgba = (rgba & 0xFFFF00FF) | (b << 8U); }
  constexpr std::uint8_t b() const { return (rgba >> 8U) & 0xFF; }

  constexpr void set_bf(const float b) { set_b(static_cast<std::uint8_t>(b * 255.0F)); }
  constexpr float bf() const { return static_cast<float>(b()) / 255.0F; }

  constexpr void set_a(const std::uint8_t a) { rgba = (rgba & 0xFFFFFF00) | a; }
  constexpr std::uint8_t a() const { return rgba & 0xFF; }

  constexpr void set_af(const float a) { set_a(static_cast<std::uint8_t>(a * 255.0F)); }
  constexpr float af() const { return static_cast<float>(a()) / 255.0F; }

  constexpr Color invert() const
  {
    return Color{static_cast<std::uint8_t>(0xFF - r()), static_cast<std::uint8_t>(0xFF - g()),
                 static_cast<std::uint8_t>(0xFF - b()), a()};
  }

  constexpr Color operator*(const float factor) const
  {
    return Color{rf() * factor, gf() * factor, bf() * factor, af()};
  }

  constexpr Color operator+(const Color& other) const
  {
    return Color{static_cast<std::uint8_t>(r() + other.r()), static_cast<std::uint8_t>(g() + other.g()),
                 static_cast<std::uint8_t>(b() + other.b()), a()};
  }

  friend inline std::ostream& operator<<(std::ostream& os, const Color color)
  {
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
    return os;
  }

  std::uint32_t rgba{0x000000FF};
};

constexpr inline Color lerp(const Color& lhs, const Color& rhs, const float t)
{
  return Color{math::lerp(lhs.rf(), rhs.rf(), t), math::lerp(lhs.gf(), rhs.gf(), t), math::lerp(lhs.bf(), rhs.bf(), t),
               math::lerp(lhs.af(), rhs.af(), t)};
}

} // namespace rtw::sw_renderer
