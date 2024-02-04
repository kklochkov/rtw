#pragma once

#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/flags.h"
#include "sw_renderer/mesh.h"
#include "sw_renderer/projection.h"
#include "sw_renderer/vertex.h"

#include "math/point.h"

namespace rtw::sw_renderer {

enum class RenderMode : std::uint8_t
{
  FaceCulling = 1U << 0U,
  Wireframe = 1U << 1U,
  Shading = 1U << 2U,
  VertexDrawing = 1U << 3U,
  Light = 1U << 4U,
  NormalDraw = 1U << 5U,
  Texture = 1U << 6U
};

using RenderModeFlags = Flags<RenderMode>;

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

  void set_face_culling_enabled(const bool enabled) { render_mode_.set(RenderMode::FaceCulling, enabled); }
  bool face_culling_enabled() const { return render_mode_ & RenderMode::FaceCulling; }
  void set_wireframe_enabled(const bool enabled) { render_mode_.set(RenderMode::Wireframe, enabled); }
  bool wireframe_enabled() const { return render_mode_ & RenderMode::Wireframe; }
  void set_shading_enabled(const bool enabled) { render_mode_.set(RenderMode::Shading, enabled); }
  bool shading_enabled() const { return render_mode_ & RenderMode::Shading; }
  void set_vertex_drawing_enabled(const bool enabled) { render_mode_.set(RenderMode::VertexDrawing, enabled); }
  bool vertex_drawing_enabled() const { return render_mode_ & RenderMode::VertexDrawing; }
  void set_light_enabled(const bool enabled) { render_mode_.set(RenderMode::Light, enabled); }
  bool light_enabled() const { return render_mode_ & RenderMode::Light; }
  void set_normal_draw_enabled(const bool enabled) { render_mode_.set(RenderMode::NormalDraw, enabled); }
  bool normal_draw_enabled() const { return render_mode_ & RenderMode::NormalDraw; }
  void set_texture_enabled(const bool enabled) { render_mode_.set(RenderMode::Texture, enabled); }
  bool texture_enabled() const { return render_mode_ & RenderMode::Texture; }

  void set_depth(const std::size_t x, const std::size_t y, const float depth) { depth_buffer_.set_depth(x, y, depth); }
  float depth(const std::size_t x, const std::size_t y) const { return depth_buffer_.depth(x, y); }

  void draw_pixel(const math::Point2i& point, const Color color);
  void draw_pixel(const math::Point2i& point, const Color color, const std::uint16_t size);
  void draw_line(const math::Point2i& p0, const math::Point2i& p1, const Color color);
  void draw_triangle(const math::Point2i& v0, const math::Point2i& v1, const math::Point2i& v2, const Color color)
  {
    draw_line(v0, v1, color);
    draw_line(v1, v2, color);
    draw_line(v2, v0, color);
  }
  void fill_triangle_bbox(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const Color color,
                          const float light_intensity);
  void fill_triangle_bbox(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const float light_intensity);
  void fill_triangle_bbox(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const Texture& texture,
                          const float light_intensity);

  void fill_triangle(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const Color color,
                     const float light_intensity);
  void fill_triangle(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const float light_intensity);
  void fill_triangle(const Vertex4f& v0, const Vertex4f& v1, const Vertex4f& v2, const Texture& texture,
                     const float light_intensity);

  void draw_mesh(const Mesh& mesh, const math::Matrix4x4f& model_view_matrix);

private:
  ColorBuffer color_buffer_;
  DepthBuffer depth_buffer_;
  Frustum3f frustum_;
  math::Matrix4x4f projection_matrix_;
  math::Matrix4x4f screen_space_matrix_;
  math::Vector3f light_direction_;
  RenderModeFlags render_mode_{RenderMode::FaceCulling | RenderMode::Wireframe | RenderMode::Shading |
                               RenderMode::Light};
};

} // namespace rtw::sw_renderer
