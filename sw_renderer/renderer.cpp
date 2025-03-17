#include "sw_renderer/renderer.h"
#include "sw_renderer/clipping.h"
#include "sw_renderer/format.h" // IWYU pragma: keep
#include "sw_renderer/projection.h"
#include "sw_renderer/rasterisation_routines.h"

#include "math/angle.h"
#include "math/barycentric.h"
#include "math/convex_polygon.h"
#include "math/format.h" // IWYU pragma: keep

#include <fmt/core.h>

namespace rtw::sw_renderer
{

using namespace math::angle_literals;

Renderer::Renderer(const std::size_t width, const std::size_t height)
    : color_buffer_(width, height), depth_buffer_(width, height)
{
  const auto aspect_ratio = color_buffer_.aspect_ratio();
  const auto fov_y = 60.0_degF;

  const auto frustum_params = make_perspective_parameters(fov_y, aspect_ratio, 0.1F, 100.0F);
  projection_matrix_ = make_perspective_projection_matrix(frustum_params);
  frustum_ = make_frustum(frustum_params);
  screen_space_matrix_ = make_screen_space_matrix<float>(width, height);
  light_direction_ = math::normalize(math::Vector3F{0.0F, 0.0F, -1.0F});

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

void Renderer::draw_pixel(const math::Point2I& point, const Color color)
{
  assert(point.x() >= 0 && point.x() < static_cast<std::int32_t>(width()));
  assert(point.y() >= 0 && point.y() < static_cast<std::int32_t>(height()));
  color_buffer_.set_pixel(point.x(), point.y(), color);
}

void Renderer::draw_pixel(const math::Point2I& point, const Color color, const std::uint16_t size)
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

void Renderer::draw_line(const math::Point2I& p0, const math::Point2I& p1, const Color color)
{
  sw_renderer::draw_line_bresenham(p0, p1, [this, color](const math::Point2I& p) { draw_pixel(p, color); });
}

void Renderer::fill_triangle_bbox(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const Color color,
                                  const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(
      v0, v1, v2,
      [this, color, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const math::Point2I& p,
                                     const math::Barycentric3F& b)
      {
        // Perspective correct interpolation of depth.
        const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
        if (inv_z < depth(p.x(), p.y()))
        {
          draw_pixel(p, color * light_intensity);
          set_depth(p.x(), p.y(), inv_z);
        }
      });
}

void Renderer::fill_triangle_bbox(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2,
                                  const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(v0, v1, v2,
                                  [this, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2,
                                                          const math::Point2I& p, const math::Barycentric3F& b)
                                  {
                                    // Perspective correct interpolation of depth.
                                    const auto inv_z =
                                        1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
                                    if (inv_z < depth(p.x(), p.y()))
                                    {
                                      // Perspective correct interpolation of color.
                                      const auto color =
                                          (v0.color * b.w0() + v1.color * b.w1() + v2.color * b.w2()) * inv_z;
                                      draw_pixel(p, color * light_intensity);
                                      set_depth(p.x(), p.y(), inv_z);
                                    }
                                  });
}

void Renderer::fill_triangle_bbox(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const Texture& texture,
                                  const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(
      v0, v1, v2,
      [this, &texture, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2,
                                        const math::Point2I& p, const math::Barycentric3F& b)
      {
        // Perspective correct interpolation of depth.
        const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
        if (inv_z < depth(p.x(), p.y()))
        {
          // Perspective correct interpolation of texture coordinates.
          const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
          const auto texel =
              texture.texel(static_cast<std::size_t>(tex_coord.u() * static_cast<float>(texture.width())),
                            static_cast<std::size_t>(tex_coord.v() * static_cast<float>(texture.height())));
          draw_pixel(p, texel * light_intensity);
          set_depth(p.x(), p.y(), inv_z);
        }
      });
}

void Renderer::fill_triangle(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const Color color,
                             const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, color, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const math::Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<float>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            draw_pixel(p, color * light_intensity);
            set_depth(p.x(), p.y(), inv_z);
          }
        }
      });
}

void Renderer::fill_triangle(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const math::Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<float>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
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

void Renderer::fill_triangle(const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2, const Texture& texture,
                             const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, &texture, light_intensity](const Vertex4F& v0, const Vertex4F& v1, const Vertex4F& v2,
                                        const math::Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<float>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            // Perspective correct interpolation of texture coordinates.
            const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
            const auto texel =
                texture.texel(static_cast<std::size_t>(tex_coord.u() * static_cast<float>(texture.width())),
                              static_cast<std::size_t>(tex_coord.v() * static_cast<float>(texture.height())));
            draw_pixel(p, texel * light_intensity);
            set_depth(p.x(), p.y(), inv_z);
          }
        }
      });
}

void Renderer::draw_mesh(const Mesh& mesh, const math::Matrix4x4F& model_view_matrix)
{
  for (const auto& face : mesh.faces)
  {
    // Transform vertices to world space.
    Vertex4F v0{model_view_matrix * mesh.vertices[face.vertex_indices[0]]};
    Vertex4F v1{model_view_matrix * mesh.vertices[face.vertex_indices[1]]};
    Vertex4F v2{model_view_matrix * mesh.vertices[face.vertex_indices[2]]};

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
    float light_intensity = 1.0F;
    if (light_enabled())
    {
      // We are in right-handed coordinate system, so we need to take the negative z axis.
      const auto normal = v0.normal;
      light_intensity = std::clamp(-math::dot(normal, light_direction_), 0.0F, 1.0F);
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
      v0.point.w() = 1.0F / v0_w;
      v1.point.w() = 1.0F / v1_w;
      v2.point.w() = 1.0F / v2_w;

      if (face_culling_enabled())
      {
        if (math::winding_order(v0.point.xy(), v1.point.xy(), v2.point.xy()) == math::WindingOrder::CLOCKWISE)
        {
          continue;
        }
      }

      if (shading_enabled())
      {
        fill_triangle_bbox(v0, v1, v2, material.diffuse, light_intensity);
      }

      if (texture_enabled() && !mesh.textures.empty())
      {
        // Flip v coordinate because the texture is upside down.
        v0.tex_coord.v() = 1.0F - v0.tex_coord.v();
        v1.tex_coord.v() = 1.0F - v1.tex_coord.v();
        v2.tex_coord.v() = 1.0F - v2.tex_coord.v();

        // Perspective correct texture mapping.
        // Pre-divide by w to avoid division in the loop.
        v0.tex_coord /= v0_w;
        v1.tex_coord /= v1_w;
        v2.tex_coord /= v2_w;

        const auto& texture = mesh.texture(material.diffuse_texture);
        fill_triangle_bbox(v0, v1, v2, texture, light_intensity);
      }

      if (wireframe_enabled())
      {
        draw_triangle(v0.point.xy().cast<std::int32_t>(), v1.point.xy().cast<std::int32_t>(),
                      v2.point.xy().cast<std::int32_t>(), Color{0x23'23'23'FF});
      }

      if (vertex_drawing_enabled())
      {
        draw_pixel(v0.point.xy().cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
        draw_pixel(v1.point.xy().cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
        draw_pixel(v2.point.xy().cast<std::int32_t>(), Color{0xFF'00'00'FF}, 5);
      }
    }
  }

#ifdef DEBUG_DRAWING
  {
    Vertex4F v0{math::Point4f(80.0F, 40.0F, 0.0F)};
    Vertex4F v1{math::Point4f(140.0F, 40.0F, 0.0F)};
    Vertex4F v2{math::Point4f(140.0F, 100.0F, 0.0F)};
    fill_triangle(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

    const auto vertex_translation = 2 * math::Vector4i{0, 50, 0, 0};
    v0.point += vertex_translation.cast<float>();
    v0.color = Color{0xFF'00'00'FF};
    v1.point += vertex_translation.cast<float>();
    v1.color = Color{0x00'FF'00'FF};
    v2.point += vertex_translation.cast<float>();
    v2.color = Color{0x00'00'FF'FF};
    fill_triangle(v0, v1, v2, 1.0F);
  }

  {
    math::Vector4i translation{100, 0, 0, 0};

    Vertex4F v0{math::Point4f(80.0F, 40.0F, 0.0F)};
    Vertex4F v1{math::Point4f(140.0F, 40.0F, 0.0F)};
    Vertex4F v2{math::Point4f(140.0F, 100.0F, 0.0F)};
    v0.point += translation.cast<float>();
    v1.point += translation.cast<float>();
    v2.point += translation.cast<float>();
    fill_triangle_bbox(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

    translation = 2 * math::Vector4i{0, 50, 0, 0};
    v0.point += translation.cast<float>();
    v0.color = Color{0xFF'00'00'FF};
    v1.point += translation.cast<float>();
    v1.color = Color{0x00'FF'00'FF};
    v2.point += translation.cast<float>();
    v2.color = Color{0x00'00'FF'FF};
    fill_triangle_bbox(v0, v1, v2, 1.0F);
  }
#endif // DEBUG_DRAWING
}

} // namespace rtw::sw_renderer
