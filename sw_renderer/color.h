#pragma once

#include "math/interpolation.h"
#include "math/vector.h"
#include "multiprecision/math.h"
#include "sw_renderer/precision.h"

#include <cstdint>
#include <type_traits>

namespace rtw::sw_renderer
{

namespace details
{

/// Clamp a uint16 to [0, 255] and cast to uint8.
constexpr std::uint8_t clamp255(const std::uint16_t v) noexcept
{
  return static_cast<std::uint8_t>(std::min(v, std::uint16_t{255}));
}

} // namespace details

/// RGBA color stored as a packed 32-bit integer in RGBA byte order (R in MSB, A in LSB).
///
/// Channel layout in memory (from most-significant to least-significant byte):
///   bits [31:24] = Red, [23:16] = Green, [15:8] = Blue, [7:0] = Alpha.
///
/// Integer channels are in [0, 255]. Float channels are in [0.0, 1.0] (clamped on input).
struct Color
{
  constexpr Color() noexcept = default;
  constexpr explicit Color(const std::uint32_t color) noexcept : rgba{color} {}

  /// Construct from unsigned integer channels [0, 255].
  template <typename T = std::uint8_t, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = 0xFF) noexcept
      : rgba(static_cast<std::uint32_t>(r) << 24U | static_cast<std::uint32_t>(g) << 16U
             | static_cast<std::uint32_t>(b) << 8U | a)
  {
  }

  /// Construct from floating-point channels [0.0, 1.0]. Values are clamped before conversion.
  template <typename T = float, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
  constexpr Color(const T r, const T g, const T b, const T a = 1.0F) noexcept
      : Color{static_cast<std::uint8_t>(multiprecision::math::saturate(r) * T{255}),
              static_cast<std::uint8_t>(multiprecision::math::saturate(g) * T{255}),
              static_cast<std::uint8_t>(multiprecision::math::saturate(b) * T{255}),
              static_cast<std::uint8_t>(multiprecision::math::saturate(a) * T{255})}
  {
  }

  constexpr explicit Color(const math::Vector4<single_precision>& vec) noexcept
      : Color{static_cast<float>(vec.x()), static_cast<float>(vec.y()), static_cast<float>(vec.z()),
              static_cast<float>(vec.w())}
  {
  }

  constexpr void set_r(const std::uint8_t r) noexcept
  {
    rgba = (rgba & 0x00'FF'FF'FFU) | (static_cast<std::uint32_t>(r) << 24U);
  }
  constexpr std::uint8_t r() const noexcept { return (rgba >> 24U) & 0xFFU; }

  constexpr void set_rf(const float r) noexcept
  {
    set_r(static_cast<std::uint8_t>(multiprecision::math::saturate(r) * 255.0F));
  }
  constexpr float rf() const noexcept { return static_cast<float>(r()) / 255.0F; }

  constexpr void set_g(const std::uint8_t g) noexcept
  {
    rgba = (rgba & 0xFF'00'FF'FFU) | (static_cast<std::uint32_t>(g) << 16U);
  }
  constexpr std::uint8_t g() const noexcept { return (rgba >> 16U) & 0xFFU; }

  constexpr void set_gf(const float g) noexcept
  {
    set_g(static_cast<std::uint8_t>(multiprecision::math::saturate(g) * 255.0F));
  }
  constexpr float gf() const noexcept { return static_cast<float>(g()) / 255.0F; }

  constexpr void set_b(const std::uint8_t b) noexcept
  {
    rgba = (rgba & 0xFF'FF'00'FFU) | (static_cast<std::uint32_t>(b) << 8U);
  }
  constexpr std::uint8_t b() const noexcept { return (rgba >> 8U) & 0xFFU; }

  constexpr void set_bf(const float b) noexcept
  {
    set_b(static_cast<std::uint8_t>(multiprecision::math::saturate(b) * 255.0F));
  }
  constexpr float bf() const noexcept { return static_cast<float>(b()) / 255.0F; }

  constexpr void set_a(const std::uint8_t a) noexcept { rgba = (rgba & 0xFF'FF'FF'00U) | a; }
  constexpr std::uint8_t a() const noexcept { return rgba & 0xFFU; }

  constexpr void set_af(const float a) noexcept
  {
    set_a(static_cast<std::uint8_t>(multiprecision::math::saturate(a) * 255.0F));
  }
  constexpr float af() const noexcept { return static_cast<float>(a()) / 255.0F; }

  constexpr explicit operator math::Vector4<single_precision>() const noexcept
  {
    return math::Vector4<single_precision>{rf(), gf(), bf(), af()};
  }

  constexpr Color invert() const noexcept
  {
    return Color{static_cast<std::uint8_t>(0xFF - r()), static_cast<std::uint8_t>(0xFF - g()),
                 static_cast<std::uint8_t>(0xFF - b()), a()};
  }

  /// Multiply color by a scalar factor. Channels are clamped to [0.0, 1.0].
  constexpr Color operator*(const float factor) const noexcept
  {
    return Color{rf() * factor, gf() * factor, bf() * factor, af()};
  }

  /// Add two colors channel-wise. Channels are clamped to [0, 255] (saturating addition).
  constexpr Color operator+(const Color& other) const noexcept
  {
    return Color{details::clamp255(static_cast<std::uint16_t>(r()) + other.r()),
                 details::clamp255(static_cast<std::uint16_t>(g()) + other.g()),
                 details::clamp255(static_cast<std::uint16_t>(b()) + other.b()), a()};
  }

  constexpr bool operator==(const Color& other) const noexcept { return rgba == other.rgba; }
  constexpr bool operator!=(const Color& other) const noexcept { return rgba != other.rgba; }

  std::uint32_t rgba{0x00'00'00'FF}; ///< Packed RGBA (R=MSB, A=LSB). Default: transparent black.
};

static_assert(sizeof(Color) == 4, "Color must be exactly 4 bytes");

constexpr Color lerp(const Color& lhs, const Color& rhs, const float t) noexcept
{
  return Color{math::lerp(lhs.rf(), rhs.rf(), t), math::lerp(lhs.gf(), rhs.gf(), t), math::lerp(lhs.bf(), rhs.bf(), t),
               math::lerp(lhs.af(), rhs.af(), t)};
}

} // namespace rtw::sw_renderer
