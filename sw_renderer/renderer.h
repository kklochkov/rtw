#pragma once

#include "stl/flags.h"
#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/mesh.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex.h"

namespace rtw::sw_renderer
{

enum class RenderMode : std::uint8_t
{
  NONE = 0U,
  FACE_CULLING = 1U << 0U,
  WIREFRAME = 1U << 1U,
  SHADING = 1U << 2U,
  VERTEX_DRAWING = 1U << 3U,
  LIGHT = 1U << 4U,
  NORMAL_DRAW = 1U << 5U,
  TEXTURE = 1U << 6U
};

using RenderModeFlags = stl::Flags<RenderMode>;

class Renderer
{
public:
  Renderer(const std::size_t width, const std::size_t height);

  std::size_t width() const { return color_buffer_.width(); }
  std::size_t height() const { return color_buffer_.height(); }
  std::size_t pitch() const { return color_buffer_.pitch(); }

  const std::uint32_t* data() const { return color_buffer_.data(); }

  void clear(const Color color)
  {
    color_buffer_.clear(color);
    depth_buffer_.clear();
  }

  void set_render_mode(const RenderModeFlags mode) { render_mode_ = mode; }
  RenderModeFlags render_mode() const { return render_mode_; }

  void set_face_culling_enabled(const bool enabled) { render_mode_.set(RenderMode::FACE_CULLING, enabled); }
  bool face_culling_enabled() const { return render_mode_.test(RenderMode::FACE_CULLING); }
  void set_wireframe_enabled(const bool enabled) { render_mode_.set(RenderMode::WIREFRAME, enabled); }
  bool wireframe_enabled() const { return render_mode_.test(RenderMode::WIREFRAME); }
  void set_shading_enabled(const bool enabled) { render_mode_.set(RenderMode::SHADING, enabled); }
  bool shading_enabled() const { return render_mode_.test(RenderMode::SHADING); }
  void set_vertex_drawing_enabled(const bool enabled) { render_mode_.set(RenderMode::VERTEX_DRAWING, enabled); }
  bool vertex_drawing_enabled() const { return render_mode_.test(RenderMode::VERTEX_DRAWING); }
  void set_light_enabled(const bool enabled) { render_mode_.set(RenderMode::LIGHT, enabled); }
  bool light_enabled() const { return render_mode_.test(RenderMode::LIGHT); }
  void set_normal_draw_enabled(const bool enabled) { render_mode_.set(RenderMode::NORMAL_DRAW, enabled); }
  bool normal_draw_enabled() const { return render_mode_.test(RenderMode::NORMAL_DRAW); }
  void set_texture_enabled(const bool enabled) { render_mode_.set(RenderMode::TEXTURE, enabled); }
  bool texture_enabled() const { return render_mode_.test(RenderMode::TEXTURE); }

  void set_depth(const std::size_t x, const std::size_t y, const float depth) { depth_buffer_.set_depth(x, y, depth); }
  float depth(const std::size_t x, const std::size_t y) const { return depth_buffer_.depth(x, y); }

  void draw_pixel(const Point2I& point, const Color color);
  void draw_pixel(const Point2I& point, const Color color, const std::uint16_t size);
  void draw_line(const Point2I& p0, const Point2I& p1, const Color color);
  void draw_triangle(const Point2I& v0, const Point2I& v1, const Point2I& v2, const Color color)
  {
    draw_line(v0, v1, color);
    draw_line(v1, v2, color);
    draw_line(v2, v0, color);
  }
  void fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Color color,
                          const float light_intensity);
  void fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const float light_intensity);
  void fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Texture& texture,
                          const float light_intensity);

  void fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Color color,
                     const float light_intensity);
  void fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const float light_intensity);
  void fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Texture& texture,
                     const float light_intensity);

  void draw_mesh(const Mesh& mesh, const Matrix4x4F& model_view_matrix);

private:
  ColorBuffer color_buffer_;
  DepthBuffer depth_buffer_;
  Frustum3F frustum_;
  Matrix4x4F projection_matrix_;
  Matrix4x4F screen_space_matrix_;
  Vector3F light_direction_;
  RenderModeFlags render_mode_{RenderMode::FACE_CULLING | RenderMode::WIREFRAME | RenderMode::SHADING
                               | RenderMode::LIGHT};
};

} // namespace rtw::sw_renderer
