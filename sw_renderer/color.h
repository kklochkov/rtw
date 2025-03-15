#pragma once

#include "math/interpolation.h"

#include <cstdint>
#include <type_traits>

namespace rtw::sw_renderer
{

struct Color
{
  constexpr Color() noexcept = default;
  constexpr explicit Color(const std::uint32_t color) noexcept : rgba{color} {}

  template <typename T = std::uint8_t, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = 0xFF) noexcept
      : rgba(static_cast<std::uint32_t>(r) << 24U | static_cast<std::uint32_t>(g) << 16U
             | static_cast<std::uint32_t>(b) << 8U | a)
  {
  }

  template <typename T = float,
            std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = T{1.0F}) noexcept
      : Color{static_cast<std::uint8_t>(r * T{255.0F}), static_cast<std::uint8_t>(g * T{255.0F}),
              static_cast<std::uint8_t>(b * T{255.0F}), static_cast<std::uint8_t>(a * T{255.0F})}
  {
  }

  constexpr void set_r(const std::uint8_t r) noexcept
  {
    rgba = (rgba & 0x00'FF'FF'FFU) | (static_cast<std::uint32_t>(r) << 24U);
  }
  constexpr std::uint8_t r() const noexcept { return (rgba >> 24U) & 0xFFU; }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr void set_rf(const T r) noexcept
  {
    set_r(static_cast<std::uint8_t>(r * T{255.0F}));
  }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr T rf() const noexcept
  {
    return static_cast<T>(r()) / T{255.0F};
  }

  constexpr void set_g(const std::uint8_t g) noexcept
  {
    rgba = (rgba & 0xFF'00'FF'FFU) | (static_cast<std::uint32_t>(g) << 16U);
  }
  constexpr std::uint8_t g() const noexcept { return (rgba >> 16U) & 0xFFU; }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr void set_gf(const T g) noexcept
  {
    set_g(static_cast<std::uint8_t>(g * T{255.0F}));
  }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr T gf() const noexcept
  {
    return static_cast<T>(g()) / T{255.0F};
  }

  constexpr void set_b(const std::uint8_t b) noexcept
  {
    rgba = (rgba & 0xFF'FF'00'FFU) | (static_cast<std::uint32_t>(b) << 8U);
  }
  constexpr std::uint8_t b() const noexcept { return (rgba >> 8U) & 0xFFU; }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr void set_bf(const T b) noexcept
  {
    set_b(static_cast<std::uint8_t>(b * T{255.0F}));
  }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr T bf() const noexcept
  {
    return static_cast<T>(b()) / T{255.0F};
  }

  constexpr void set_a(const std::uint8_t a) noexcept { rgba = (rgba & 0xFF'FF'FF'00U) | a; }
  constexpr std::uint8_t a() const noexcept { return rgba & 0xFFU; }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr void set_af(const T a) noexcept
  {
    set_a(static_cast<std::uint8_t>(a * T{255.0F}));
  }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr T af() const noexcept
  {
    return static_cast<T>(a()) / T{255.0F};
  }

  constexpr Color invert() const noexcept
  {
    return Color{static_cast<std::uint8_t>(0xFF - r()), static_cast<std::uint8_t>(0xFF - g()),
                 static_cast<std::uint8_t>(0xFF - b()), a()};
  }

  template <typename T = float,
            typename = std::enable_if_t<std::is_floating_point_v<T> || fixed_point::IS_FIXED_POINT_V<T>>>
  constexpr Color operator*(const T factor) const noexcept
  {
    return Color{rf<T>() * factor, gf<T>() * factor, bf<T>() * factor, af<T>()};
  }

  constexpr Color operator+(const Color& other) const noexcept
  {
    return Color{static_cast<std::uint8_t>(r() + other.r()), static_cast<std::uint8_t>(g() + other.g()),
                 static_cast<std::uint8_t>(b() + other.b()), a()};
  }

  friend std::ostream& operator<<(std::ostream& os, const Color color) noexcept;

  std::uint32_t rgba{0x00'00'00'FF};
};

template <typename T>
constexpr Color lerp(const Color& lhs, const Color& rhs, const T t) noexcept
{
  return Color{math::lerp(lhs.rf<T>(), rhs.rf<T>(), t), math::lerp(lhs.gf<T>(), rhs.gf<T>(), t),
               math::lerp(lhs.bf<T>(), rhs.bf<T>(), t), math::lerp(lhs.af<T>(), rhs.af<T>(), t)};
}

} // namespace rtw::sw_renderer
