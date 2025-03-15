#pragma once

#include "sw_renderer/clipping.h"
#include "sw_renderer/flags.h"
#include "sw_renderer/format.h"
#include "sw_renderer/mesh.h"
#include "sw_renderer/projection.h"
#include "sw_renderer/renderer2d.h"
#include "sw_renderer/vertex.h"

#include "math/angle.h"
#include "math/format.h"
#include "math/point.h"

#include <fmt/core.h>

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

using RenderModeFlags = Flags<RenderMode>;

template <typename T>
class GenericRenderer3d
{
public:
  GenericRenderer3d(const std::size_t width, const std::size_t height)
      : renderer2d_(width, height), screen_space_matrix_(make_screen_space_matrix<T>(width, height)),
        light_direction_(math::normalize(math::Vector3<T>{0.0F, 0.0F, -1.0F}))
  {
    const auto aspect_ratio = renderer2d_.aspect_ratio();
    const math::Angle<T> fov_y{math::DEG, 60.0F};

    const auto frustum_params = make_perspective_parameters<T>(fov_y, aspect_ratio, 0.1F, 100.0F);
    projection_matrix_ = make_perspective_projection_matrix(frustum_params);
    frustum_ = make_frustum(frustum_params);

    fmt::print("projection_matrix_: {}\n", projection_matrix_);
    fmt::print("screen_space_matrix_: {}\n", screen_space_matrix_);

    const auto f = extract_frustum(projection_matrix_);
    fmt::print("frustum_.left.normal: {}\n", frustum_.left.normal);
    fmt::print("f.left.normal: {}\n", f.left.normal);
    fmt::print("frustum_.left.distance: {}\n", frustum_.left.distance);
    fmt::print("f.left.distance: {}\n", f.left.distance);

    fmt::print("frustum_.right.normal: {}\n", frustum_.right.normal);
    fmt::print("f.right.normal: {}\n", f.right.normal);
    fmt::print("frustum_.right.distance: {}\n", frustum_.right.distance);
    fmt::print("f.right.distance: {}\n", f.right.distance);

    fmt::print("frustum_.top.normal: {}\n", frustum_.top.normal);
    fmt::print("f.top.normal: {}\n", f.top.normal);
    fmt::print("frustum_.top.distance: {}\n", frustum_.top.distance);
    fmt::print("f.top.distance: {}\n", f.top.distance);

    fmt::print("frustum_.bottom.normal: {}\n", frustum_.bottom.normal);
    fmt::print("f.bottom.normal: {}\n", f.bottom.normal);
    fmt::print("frustum_.bottom.distance: {}\n", frustum_.bottom.distance);
    fmt::print("f.bottom.distance: {}\n", f.bottom.distance);

    fmt::print("frustum_.near.normal: {}\n", frustum_.near.normal);
    fmt::print("f.near.normal: {}\n", f.near.normal);
    fmt::print("frustum_.near.distance: {}\n", frustum_.near.distance);
    fmt::print("f.near.distance: {}\n", f.near.distance);

    fmt::print("frustum_.far.normal: {}\n", frustum_.far.normal);
    fmt::print("f.far.normal: {}\n", f.far.normal);
    fmt::print("frustum_.far.distance: {}\n", frustum_.far.distance);
    fmt::print("f.far.distance: {}\n", f.far.distance);
  }

  std::size_t width() const { return renderer2d_.width(); }
  std::size_t height() const { return renderer2d_.height(); }
  std::size_t pitch() const { return renderer2d_.pitch(); }

  const std::uint32_t* data() const { return renderer2d_.data(); }

  void clear(const Color color) { renderer2d_.clear(color); }

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

  void draw_mesh(const GenericMesh<T>& mesh, const math::Matrix4x4<T>& model_view_matrix)
  {
    for (const auto& face : mesh.faces)
    {
      // Transform vertices to world space.
      Vertex4<T> v0{model_view_matrix * mesh.vertices[face.vertex_indices[0]]};
      Vertex4<T> v1{model_view_matrix * mesh.vertices[face.vertex_indices[1]]};
      Vertex4<T> v2{model_view_matrix * mesh.vertices[face.vertex_indices[2]]};

      if (face.texture_indices.has_value() && !mesh.textures.empty())
      {
        v0.tex_coord = mesh.tex_coords[(*face.texture_indices)[0]];
        v1.tex_coord = mesh.tex_coords[(*face.texture_indices)[1]];
        v2.tex_coord = mesh.tex_coords[(*face.texture_indices)[2]];
      }

      if (face.normal_indices.has_value() && !mesh.normals.empty())
      {
        // Transform normals to world space.
        v0.normal = (model_view_matrix * mesh.normals[(*face.normal_indices)[0]]).xyz();
        v1.normal = (model_view_matrix * mesh.normals[(*face.normal_indices)[1]]).xyz();
        v2.normal = (model_view_matrix * mesh.normals[(*face.normal_indices)[2]]).xyz();
      }
      else
      {
        // Calculate face normal.
        // We are in right-handed coordinate system, so we need to use counter-clockwise winding order.
        v0.normal = math::normalize(math::cross((v1.point - v0.point).xyz(), (v2.point - v0.point).xyz()));
        v1.normal = v1.normal;
        v2.normal = v1.normal;
      }

      // Lighting.
      const Material material = mesh.material(face.material);
      T light_intensity{1.0F};
      if (light_enabled())
      {
        // We are in right-handed coordinate system, so we need to take the negative z axis.
        const auto normal = v0.normal;

        using rtw::fixed_point::math::clamp;
        using std::clamp;

        light_intensity = clamp(-math::dot(normal, light_direction_), T{0.0F}, T{1.0F});
      }

      // Frustum clipping.
      const auto polygon = clip(v0, v1, v2, frustum_);
      const auto triangles = triangulate(polygon);
      for (std::size_t i = 0U; i < triangles.triangle_count; ++i)
      {
        const auto& triangle = triangles.triangles[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

        v0 = triangle[0U];
        v1 = triangle[1U];
        v2 = triangle[2U];

        // Transform to screen space.
        v0.point = projection_matrix_ * v0.point;
        v1.point = projection_matrix_ * v1.point;
        v2.point = projection_matrix_ * v2.point;

        const auto v0_w = v0.point.w();
        const auto v1_w = v1.point.w();
        const auto v2_w = v2.point.w();

        // Perspective divide.
        v0.point /= v0_w;
        v1.point /= v1_w;
        v2.point /= v2_w;

        // Transform to screen space.
        v0.point = ndc_to_screen_space(v0.point, screen_space_matrix_);
        v1.point = ndc_to_screen_space(v1.point, screen_space_matrix_);
        v2.point = ndc_to_screen_space(v2.point, screen_space_matrix_);

        // Pre-compute reciprocal w for each vertex to avoid division in the loop.
        v0.point.w() = T{1.0F} / v0_w;
        v1.point.w() = T{1.0F} / v1_w;
        v2.point.w() = T{1.0F} / v2_w;

        if (face_culling_enabled())
        {
          if (winding_order(v0.point.xy(), v1.point.xy(), v2.point.xy()) == WindingOrder::CLOCKWISE)
          {
            continue;
          }
        }

        if (shading_enabled())
        {
          renderer2d_.fill_triangle_bbox(v0, v1, v2, material.diffuse, light_intensity);
        }

        if (texture_enabled() && !mesh.textures.empty())
        {
          // Flip v coordinate because the texture is upside down.
          v0.tex_coord.v() = T{1.0F} - v0.tex_coord.v();
          v1.tex_coord.v() = T{1.0F} - v1.tex_coord.v();
          v2.tex_coord.v() = T{1.0F} - v2.tex_coord.v();

          // Perspective correct texture mapping.
          // Pre-divide by w to avoid division in the loop.
          v0.tex_coord /= v0_w;
          v1.tex_coord /= v1_w;
          v2.tex_coord /= v2_w;

          const auto& texture = mesh.texture(material.diffuse_texture);
          renderer2d_.fill_triangle_bbox(v0, v1, v2, texture, light_intensity);
        }

        if (wireframe_enabled())
        {
          renderer2d_.draw_triangle(v0.point.xy().template cast<std::int32_t>(),
                                    v1.point.xy().template cast<std::int32_t>(),
                                    v2.point.xy().template cast<std::int32_t>(), Color{0x23'23'23'FF});
        }

        if (vertex_drawing_enabled())
        {
          renderer2d_.draw_pixel(v0.point.xy().template cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
          renderer2d_.draw_pixel(v1.point.xy().template cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
          renderer2d_.draw_pixel(v2.point.xy().template cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
        }
      }
    }

#ifdef DEBUG_DRAWING
    {
      Vertex4<T> v0{math::Point4<T>(80.0F, 40.0F, 0.0F)};
      Vertex4<T> v1{math::Point4<T>(140.0F, 40.0F, 0.0F)};
      Vertex4<T> v2{math::Point4<T>(140.0F, 100.0F, 0.0F)};
      renderer2d_.fill_triangle(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

      const auto vertex_translation = 2 * math::Vector4i{0, 50, 0, 0};
      v0.point += vertex_translation.cast<T>();
      v0.color = Color{0xFF'00'00'FF};
      v1.point += vertex_translation.cast<T>();
      v1.color = Color{0x00'FF'00'FF};
      v2.point += vertex_translation.cast<T>();
      v2.color = Color{0x00'00'FF'FF};
      renderer2d_.fill_triangle(v0, v1, v2, 1.0F);
    }

    {
      math::Vector4i translation{100, 0, 0, 0};

      Vertex4<T> v0{math::Point4<T>(80.0F, 40.0F, 0.0F)};
      Vertex4<T> v1{math::Point4<T>(140.0F, 40.0F, 0.0F)};
      Vertex4<T> v2{math::Point4<T>(140.0F, 100.0F, 0.0F)};
      v0.point += translation.cast<T>();
      v1.point += translation.cast<T>();
      v2.point += translation.cast<T>();
      renderer2d_.fill_triangle_bbox(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

      translation = 2 * math::Vector4i{0, 50, 0, 0};
      v0.point += translation.cast<T>();
      v0.color = Color{0xFF'00'00'FF};
      v1.point += translation.cast<T>();
      v1.color = Color{0x00'FF'00'FF};
      v2.point += translation.cast<T>();
      v2.color = Color{0x00'00'FF'FF};
      renderer2d_.fill_triangle_bbox(v0, v1, v2, 1.0F);
    }
#endif // DEBUG_DRAWING
  }

private:
  GenericRenderer2d<T> renderer2d_;
  Frustum3<T> frustum_;
  math::Matrix4x4<T> projection_matrix_;
  math::Matrix4x4<T> screen_space_matrix_;
  math::Vector3<T> light_direction_;
  RenderModeFlags render_mode_{RenderMode::FACE_CULLING | RenderMode::WIREFRAME | RenderMode::SHADING
                               | RenderMode::LIGHT};
};

using Renderer3d = GenericRenderer3d<float>;
using Renderer3dQ16 = GenericRenderer3d<fixed_point::FixedPoint16>;
using Renderer3dQ32 = GenericRenderer3d<fixed_point::FixedPoint32>;

} // namespace rtw::sw_renderer
