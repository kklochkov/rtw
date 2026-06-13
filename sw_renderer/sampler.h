#pragma once

#include "sw_renderer/color.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include "math/interpolation.h"

#include "multiprecision/fixed_point_math.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace rtw::sw_renderer
{

/// Texture coordinate wrapping mode, applied per-axis when sampling outside [0, 1].
enum class WrapMode : std::uint8_t
{
  REPEAT,          ///< Tile the texture (GL_REPEAT).
  CLAMP_TO_EDGE,   ///< Clamp to the nearest edge texel (GL_CLAMP_TO_EDGE).
  MIRRORED_REPEAT, ///< Tile with mirroring on every other repeat (GL_MIRRORED_REPEAT).
};

/// Texture minification / magnification filter.
enum class FilterMode : std::uint8_t
{
  NEAREST = 0U, ///< Nearest-neighbor sampling (GL_NEAREST).
  LINEAR = 1U,  ///< Bilinear interpolation of the four nearest texels (GL_LINEAR).
};

/// A 2D texture sampler over a (non-owning) Texture. Applies wrapping and filtering and returns
/// normalized RGBA in [0, 1]. Texture coordinates use the top-left origin (+v points down).
class Sampler2D
{
public:
  constexpr Sampler2D() = default;
  constexpr explicit Sampler2D(const Texture& texture, const WrapMode wrap = WrapMode::REPEAT,
                               FilterMode filter = FilterMode::NEAREST) noexcept
      : texture_{&texture}, wrap_{wrap}, filter_{filter}
  {
  }

  constexpr void set_texture(const Texture& texture) noexcept { texture_ = &texture; }
  constexpr const Texture* get_texture() const noexcept { return texture_; }

  constexpr void set_wrap_mode(const WrapMode wrap) noexcept { wrap_ = wrap; }
  constexpr WrapMode get_wrap_mode() const noexcept { return wrap_; }

  constexpr void set_filter_mode(const FilterMode filter) noexcept { filter_ = filter; }
  constexpr FilterMode get_filter_mode() const noexcept { return filter_; }

  constexpr Vector4F sample(const Vector2F& uv) const
  {
    assert(texture_ != nullptr && "Sampler2D: texture must be set before sampling.");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return SAMPLERS[static_cast<std::size_t>(filter_)](*texture_, uv, wrap_);
  }

  constexpr Vector4F sample(const single_precision u, const single_precision v) const { return sample(Vector2F{u, v}); }

private:
  constexpr static std::int64_t floor_to_int(const single_precision value) noexcept
  {
    using multiprecision::math::floor;
    using std::floor;
    return static_cast<std::int64_t>(floor(value));
  }

  constexpr static std::size_t wrap_coordinate(const std::int64_t coord, const std::size_t size,
                                               const WrapMode mode) noexcept
  {
    const auto signed_size = static_cast<std::int64_t>(size);
    switch (mode)
    {
    case WrapMode::REPEAT:
      return static_cast<std::size_t>(((coord % signed_size) + signed_size) % signed_size);
    case WrapMode::MIRRORED_REPEAT:
    {
      const auto period = 2 * signed_size;
      const auto mod = ((coord % period) + period) % period;
      const std::array<std::int64_t, 2U> mirror = {(period - mod - 1), mod};
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      return static_cast<std::size_t>(mirror[static_cast<std::size_t>(mod < signed_size)]);
    }
    case WrapMode::CLAMP_TO_EDGE:
      break;
    }

    return static_cast<std::size_t>(std::clamp(coord, std::int64_t{0}, static_cast<std::int64_t>(size - 1U)));
  }

  constexpr static Vector4F fetch(const Texture& texture, const std::int64_t x, const std::int64_t y,
                                  const WrapMode mode)
  {
    const auto wrapped_x = wrap_coordinate(x, texture.width(), mode);
    const auto wrapped_y = wrap_coordinate(y, texture.height(), mode);
    return static_cast<Vector4F>(texture.texel(wrapped_x, wrapped_y));
  }

  constexpr static Vector4F sample_nearest(const Texture& texture, const Vector2F& uv, const WrapMode mode)
  {
    const auto x = floor_to_int(uv.x() * static_cast<single_precision>(texture.width()));
    const auto y = floor_to_int(uv.y() * static_cast<single_precision>(texture.height()));
    return fetch(texture, x, y, mode);
  }

  constexpr static Vector4F sample_linear(const Texture& texture, const Vector2F& uv, const WrapMode mode)
  {
    // Shift by half a texel so integer coordinates land on texel centers.
    const auto x = uv.x() * static_cast<single_precision>(texture.width()) - single_precision{0.5F};
    const auto y = uv.y() * static_cast<single_precision>(texture.height()) - single_precision{0.5F};

    const auto x0 = floor_to_int(x);
    const auto y0 = floor_to_int(y);
    const auto x1 = x0 + 1U;
    const auto y1 = y0 + 1U;

    const auto sx = x - static_cast<single_precision>(x0);
    const auto sy = y - static_cast<single_precision>(y0);

    const auto c00 = fetch(texture, x0, y0, mode);
    const auto c10 = fetch(texture, x1, y0, mode);
    const auto c01 = fetch(texture, x0, y1, mode);
    const auto c11 = fetch(texture, x1, y1, mode);

    const auto top = math::lerp(c00, c10, sx);
    const auto bottom = math::lerp(c01, c11, sx);
    return math::lerp(top, bottom, sy);
  }

  constexpr static std::array SAMPLERS = {&Sampler2D::sample_nearest, &Sampler2D::sample_linear};

  const Texture* texture_{nullptr};
  WrapMode wrap_{WrapMode::REPEAT};
  FilterMode filter_{FilterMode::NEAREST};
};

} // namespace rtw::sw_renderer
