#pragma once

#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/rasterisation_routines.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/vertex.h"

#include "math/point.h"

#include <cassert>

namespace rtw::sw_renderer
{

template <typename T>
class GenericRenderer2d
{
public:
  GenericRenderer2d(const std::size_t width, const std::size_t height)
      : color_buffer_(width, height), depth_buffer_(width, height)
  {
  }

  std::size_t width() const { return color_buffer_.width(); }
  std::size_t height() const { return color_buffer_.height(); }
  float aspect_ratio() const { return color_buffer_.aspect_ratio(); }
  std::size_t pitch() const { return color_buffer_.pitch(); }

  const std::uint32_t* data() const { return color_buffer_.data(); }

  void clear(const Color color)
  {
    color_buffer_.clear(color);
    depth_buffer_.clear();
  }

  void set_depth(const std::size_t x, const std::size_t y, const T depth) { depth_buffer_.set_depth(x, y, depth); }
  T depth(const std::size_t x, const std::size_t y) const { return depth_buffer_.depth(x, y); }

  void draw_pixel(const math::Point2I& point, const Color color)
  {
    if ((point.x() >= 0) && (point.x() < static_cast<std::int32_t>(width())) && (point.y() >= 0)
        && (point.y() < static_cast<std::int32_t>(height())))
    {
      color_buffer_.set_pixel(point.x(), point.y(), color);
    }
  }

  void draw_pixel(const math::Point2I& point, const Color color, const std::uint16_t size)
  {
    const math::Point2I p0{point.x() - size / 2, point.y() - size / 2};
    const auto w = static_cast<std::int32_t>(width()) - 1;
    const auto h = static_cast<std::int32_t>(height()) - 1;
    for (std::uint16_t i = 0U; i < size; ++i)
    {
      for (std::uint16_t j = 0U; j < size; ++j)
      {
        auto p1 = p0 + math::Vector2I{i, j};
        p1.x() = std::clamp(p1.x(), 0, w);
        p1.y() = std::clamp(p1.y(), 0, h);
        draw_pixel(p1, color);
      }
    }
  }
  void draw_line(const math::Point2I& p0, const math::Point2I& p1, const Color color)
  {
    sw_renderer::draw_line_bresenham(p0, p1, [this, color](const math::Point2I& p) { draw_pixel(p, color); });
  }

  void draw_triangle(const math::Point2I& v0, const math::Point2I& v1, const math::Point2I& v2, const Color color)
  {
    draw_line(v0, v1, color);
    draw_line(v1, v2, color);
    draw_line(v2, v0, color);
  }

  void fill_triangle_bbox(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const Color color,
                          const T light_intensity)
  {
    sw_renderer::fill_triangle_bbox(
        v0, v1, v2,
        [this, color, light_intensity](const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                       const math::Point2I& p, const Barycentric3<T>& b)
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            draw_pixel(p, color * light_intensity);
            set_depth(p.x(), p.y(), inv_z);
          }
        });
  }

  void fill_triangle_bbox(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const T light_intensity)
  {
    sw_renderer::fill_triangle_bbox(
        v0, v1, v2,
        [this, light_intensity](const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                const math::Point2I& p, const Barycentric3<T>& b)
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            // Perspective correct interpolation of color.
            const auto color = (v0.color * b.w0() + v1.color * b.w1() + v2.color * b.w2()) * inv_z;
            draw_pixel(p, color * light_intensity);
            set_depth(p.x(), p.y(), inv_z);
          }
        });
  }

  void fill_triangle_bbox(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const Texture& texture,
                          const T light_intensity)
  {
    sw_renderer::fill_triangle_bbox(
        v0, v1, v2,
        [this, &texture, light_intensity](const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T> v2,
                                          const math::Point2I& p, const Barycentric3<T>& b)
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            // Perspective correct interpolation of texture coordinates.
            const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
            const auto texel =
                texture.texel(static_cast<std::size_t>(tex_coord.u() * static_cast<T>(texture.width())),
                              static_cast<std::size_t>(tex_coord.v() * static_cast<T>(texture.height())));
            draw_pixel(p, texel * light_intensity);
            set_depth(p.x(), p.y(), inv_z);
          }
        });
  }

  void fill_triangle(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const Color color,
                     const T light_intensity)
  {
    sw_renderer::fill_triangle_scanline(
        v0, v1, v2,
        [this, color, light_intensity](const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                       const math::Point2I& p)
        {
          const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<T>());
          if (contains(b))
          {
            // Perspective correct interpolation of depth.
            const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
            if (inv_z < depth(p.x(), p.y()))
            {
              draw_pixel(p, color * light_intensity);
              set_depth(p.x(), p.y(), inv_z);
            }
          }
        });
  }

  void fill_triangle(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const T light_intensity)
  {
    sw_renderer::fill_triangle_scanline(
        v0, v1, v2,
        [this, light_intensity](const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                const math::Point2I& p)
        {
          const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.template cast<T>());
          if (contains(b))
          {
            // Perspective correct interpolation of depth.
            const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
            if (inv_z < depth(p.x(), p.y()))
            {
              // Perspective correct interpolation of color.
              const auto color = (v0.color * b.w0() + v1.color * b.w1() + v2.color * b.w2()) * inv_z;
              draw_pixel(p, color * light_intensity);
              set_depth(p.x(), p.y(), inv_z);
            }
          }
#ifdef DEBUG_DRAWING
          else
          {
            draw_pixel(p, Color{0x00'00'00'FF});
          }
#endif // DEBUG_DRAWING
        });
  }

  void fill_triangle(const Vertex4<T>& v0, const Vertex4<T>& v1, const Vertex4<T>& v2, const Texture& texture,
                     const T light_intensity)
  {
    sw_renderer::fill_triangle_scanline(
        v0, v1, v2,
        [this, &texture, light_intensity](const Vertex4<T> v0, const Vertex4<T>& v1, const Vertex4<T>& v2,
                                          const math::Point2I& p)
        {
          const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.template cast<T>());
          if (contains(b))
          {
            // Perspective correct interpolation of depth.
            const auto inv_z = T{1} / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
            if (inv_z < depth(p.x(), p.y()))
            {
              // Perspective correct interpolation of texture coordinates.
              const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
              const auto texel =
                  texture.texel(static_cast<std::size_t>(tex_coord.u() * static_cast<T>(texture.width())),
                                static_cast<std::size_t>(tex_coord.v() * static_cast<T>(texture.height())));
              draw_pixel(p, texel * light_intensity);
              set_depth(p.x(), p.y(), inv_z);
            }
          }
        });
  }

private:
  ColorBuffer color_buffer_;
  GenericDepthBuffer<T> depth_buffer_;
};

using Renderer2d = GenericRenderer2d<float>;
using Renderer2dQ16 = GenericRenderer2d<fixed_point::FixedPoint16>;
using Renderer2dQ32 = GenericRenderer2d<fixed_point::FixedPoint32>;

} // namespace rtw::sw_renderer
