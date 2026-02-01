#include "sw_renderer/renderer.h"
#include "sw_renderer/clipping.h"
#include "sw_renderer/projection.h"
#include "sw_renderer/rasterisation_routines.h"

#include "math/angle.h"
#include "math/barycentric.h"
#include "math/barycentric_operations.h"
#include "math/convex_polygon.h"
#include "math/convex_polygon_operations.h"
#include "math/matrix_operations.h"
#include "math/vector_operations.h"

namespace rtw::sw_renderer
{

using namespace angle_literals;

Renderer::Renderer(const std::size_t width, const std::size_t height)
    : color_buffer_(width, height), depth_buffer_(width, height)
{
  const auto aspect_ratio = static_cast<single_precision>(color_buffer_.aspect_ratio());
  const auto fov_y = 60.0_degF;

  const auto frustum_params =
      make_perspective_parameters(fov_y, aspect_ratio, single_precision{0.1F}, single_precision{100.0F});
  projection_matrix_ = make_perspective_projection_matrix(frustum_params);
  frustum_ = make_frustum(frustum_params);
  screen_space_matrix_ = make_screen_space_matrix<single_precision>(width, height);
  light_direction_ = math::normalize(Vector3F{0.0F, 0.0F, -1.0F});
}

void Renderer::draw_pixel(const Point2I& point, const Color color)
{
  assert(point.x() >= 0 && point.x() < static_cast<std::int32_t>(width()));
  assert(point.y() >= 0 && point.y() < static_cast<std::int32_t>(height()));
  color_buffer_.set_pixel(point.x(), point.y(), color);
}

void Renderer::draw_pixel(const Point2I& point, const Color color, const std::uint16_t size)
{
  const Point2I p0{point.x() - (size / 2), point.y() - (size / 2)};
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

void Renderer::draw_line(const Point2I& p0, const Point2I& p1, const Color color)
{
  sw_renderer::draw_line_bresenham(p0, p1, [this, color](const Point2I& p) { draw_pixel(p, color); });
}

void Renderer::fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Color color,
                                  const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(
      v0, v1, v2,
      [this, color, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2, const Point2I& p,
                                     const BarycentricF& b)
      {
        // Perspective correct interpolation of depth.
        const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
        if (inv_z < depth(p.x(), p.y()))
        {
          draw_pixel(p, color * light_intensity);
          set_depth(p.x(), p.y(), static_cast<float>(inv_z));
        }
      });
}

void Renderer::fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(v0, v1, v2,
                                  [this, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2,
                                                          const Point2I& p, const BarycentricF& b)
                                  {
                                    // Perspective correct interpolation of depth.
                                    const auto inv_z =
                                        1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
                                    if (inv_z < depth(p.x(), p.y()))
                                    {
                                      // Perspective correct interpolation of color.
                                      const auto w0 = static_cast<float>(b.w0());
                                      const auto w1 = static_cast<float>(b.w1());
                                      const auto w2 = static_cast<float>(b.w2());
                                      const auto inv_z_f = static_cast<float>(inv_z);
                                      const auto color = (v0.color * w0 + v1.color * w1 + v2.color * w2) * inv_z_f;
                                      draw_pixel(p, color * light_intensity);
                                      set_depth(p.x(), p.y(), inv_z_f);
                                    }
                                  });
}

void Renderer::fill_triangle_bbox(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Texture& texture,
                                  const float light_intensity)
{
  sw_renderer::fill_triangle_bbox(
      v0, v1, v2,
      [this, &texture, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2, const Point2I& p,
                                        const BarycentricF& b)
      {
        // Perspective correct interpolation of depth.
        const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
        if (inv_z < depth(p.x(), p.y()))
        {
          // Perspective correct interpolation of texture coordinates.
          const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
          const auto texel =
              texture.texel(static_cast<std::size_t>(tex_coord.u() * static_cast<single_precision>(texture.width())),
                            static_cast<std::size_t>(tex_coord.v() * static_cast<single_precision>(texture.height())));
          draw_pixel(p, texel * light_intensity);
          set_depth(p.x(), p.y(), static_cast<float>(inv_z));
        }
      });
}

void Renderer::fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Color color,
                             const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, color, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2, const Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<single_precision>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            draw_pixel(p, color * light_intensity);
            set_depth(p.x(), p.y(), static_cast<float>(inv_z));
          }
        }
      });
}

void Renderer::fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2, const Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<single_precision>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            // Perspective correct interpolation of color.
            const auto w0 = static_cast<float>(b.w0());
            const auto w1 = static_cast<float>(b.w1());
            const auto w2 = static_cast<float>(b.w2());
            const auto inv_z_f = static_cast<float>(inv_z);
            const auto color = (v0.color * w0 + v1.color * w1 + v2.color * w2) * inv_z_f;
            draw_pixel(p, color * light_intensity);
            set_depth(p.x(), p.y(), inv_z_f);
          }
        }
#ifdef RTW_DEBUG_DRAWING
        else
        {
          draw_pixel(p, Color{0x00'00'00'FF});
        }
#endif // RTW_DEBUG_DRAWING
      });
}

void Renderer::fill_triangle(const VertexF& v0, const VertexF& v1, const VertexF& v2, const Texture& texture,
                             const float light_intensity)
{
  sw_renderer::fill_triangle_scanline(
      v0, v1, v2,
      [this, &texture, light_intensity](const VertexF& v0, const VertexF& v1, const VertexF& v2, const Point2I& p)
      {
        const auto b = make_barycentric(v0.point.xy(), v1.point.xy(), v2.point.xy(), p.cast<single_precision>());
        if (contains(b))
        {
          // Perspective correct interpolation of depth.
          const auto inv_z = 1.0F / (v0.point.w() * b.w0() + v1.point.w() * b.w1() + v2.point.w() * b.w2());
          if (inv_z < depth(p.x(), p.y()))
          {
            // Perspective correct interpolation of texture coordinates.
            const auto tex_coord = (v0.tex_coord * b.w0() + v1.tex_coord * b.w1() + v2.tex_coord * b.w2()) * inv_z;
            const auto texel = texture.texel(
                static_cast<std::size_t>(tex_coord.u() * static_cast<single_precision>(texture.width())),
                static_cast<std::size_t>(tex_coord.v() * static_cast<single_precision>(texture.height())));
            draw_pixel(p, texel * light_intensity);
            set_depth(p.x(), p.y(), static_cast<float>(inv_z));
          }
        }
      });
}

void Renderer::draw_mesh(const Mesh& mesh, const Matrix4x4F& model_view_matrix)
{
  using multiprecision::math::clamp;
  using std::clamp;

  constexpr single_precision ZERO{0.0F};
  constexpr single_precision ONE{1.0F};

  for (const auto& face : mesh.faces)
  {
    // Transform vertices to world space.
    VertexF v0{model_view_matrix * mesh.vertices[face.vertex_indices[0]]};
    VertexF v1{model_view_matrix * mesh.vertices[face.vertex_indices[1]]};
    VertexF v2{model_view_matrix * mesh.vertices[face.vertex_indices[2]]};

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
      light_intensity = static_cast<float>(clamp(-math::dot(normal, light_direction_), ZERO, ONE));
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
      v0.point.w() = ONE / v0_w;
      v1.point.w() = ONE / v1_w;
      v2.point.w() = ONE / v2_w;

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
        v0.tex_coord.v() = ONE - v0.tex_coord.v();
        v1.tex_coord.v() = ONE - v1.tex_coord.v();
        v2.tex_coord.v() = ONE - v2.tex_coord.v();

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

#ifdef RTW_DEBUG_DRAWING
  {
    VertexF v0{Point4F(80.0F, 40.0F, 0.0F)};
    VertexF v1{Point4F(140.0F, 40.0F, 0.0F)};
    VertexF v2{Point4F(140.0F, 100.0F, 0.0F)};
    fill_triangle(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

    const auto vertex_translation = 2 * math::Vector4I{0, 50, 0, 0};
    v0.point += vertex_translation.cast<single_precision>();
    v0.color = Color{0xFF'00'00'FF};
    v1.point += vertex_translation.cast<single_precision>();
    v1.color = Color{0x00'FF'00'FF};
    v2.point += vertex_translation.cast<single_precision>();
    v2.color = Color{0x00'00'FF'FF};
    fill_triangle(v0, v1, v2, 1.0F);
  }

  {
    math::Vector4I translation{100, 0, 0, 0};

    VertexF v0{Point4F(80.0F, 40.0F, 0.0F)};
    VertexF v1{Point4F(140.0F, 40.0F, 0.0F)};
    VertexF v2{Point4F(140.0F, 100.0F, 0.0F)};
    v0.point += translation.cast<single_precision>();
    v1.point += translation.cast<single_precision>();
    v2.point += translation.cast<single_precision>();
    fill_triangle_bbox(v0, v1, v2, Color{0x00'00'FF'FF}, 1.0F);

    translation = 2 * math::Vector4I{0, 50, 0, 0};
    v0.point += translation.cast<single_precision>();
    v0.color = Color{0xFF'00'00'FF};
    v1.point += translation.cast<single_precision>();
    v1.color = Color{0x00'FF'00'FF};
    v2.point += translation.cast<single_precision>();
    v2.color = Color{0x00'00'FF'FF};
    fill_triangle_bbox(v0, v1, v2, 1.0F);
  }
#endif // RTW_DEBUG_DRAWING
}

} // namespace rtw::sw_renderer
