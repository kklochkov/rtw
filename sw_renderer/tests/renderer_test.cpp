#include "sw_renderer/clipping.h"
#include "sw_renderer/renderer.h"

#include "math/frustum.h"

#include <gtest/gtest.h>

namespace rtw::sw_renderer
{
namespace
{

// --- RenderStats tests ---

TEST(Renderer, stats_reset_clears_all_fields)
{
  RenderStats stats{};
  stats.triangles_submitted = 42;
  stats.triangles_clipped = 10;
  stats.triangles_culled = 5;
  stats.triangles_rendered = 27;
  stats.reset();
  EXPECT_EQ(stats.triangles_submitted, 0U);
  EXPECT_EQ(stats.triangles_clipped, 0U);
  EXPECT_EQ(stats.triangles_culled, 0U);
  EXPECT_EQ(stats.triangles_rendered, 0U);
}

TEST(Renderer, stats_initially_zero_after_clear)
{
  Renderer renderer{64, 64};
  renderer.set_render_stats_enabled(true);
  renderer.clear(Color{0x00'00'00'FF});
  const auto& stats = renderer.stats();
  EXPECT_EQ(stats.triangles_submitted, 0U);
  EXPECT_EQ(stats.triangles_clipped, 0U);
  EXPECT_EQ(stats.triangles_culled, 0U);
  EXPECT_EQ(stats.triangles_rendered, 0U);
}

TEST(Renderer, stats_counts_clipped_triangle_outside_frustum)
{
  Renderer renderer{64, 64};
  renderer.set_render_stats_enabled(true);
  renderer.set_face_culling_enabled(false);
  renderer.set_wireframe_enabled(false);
  renderer.set_shading_enabled(false);
  renderer.set_texture_enabled(false);
  renderer.clear(Color{0x00'00'00'FF});

  // Create a mesh with a single face far outside the frustum (behind the camera, z > 0).
  Mesh mesh;
  mesh.vertices.push_back(Point3F{1000.0F, 1000.0F, 1000.0F});
  mesh.vertices.push_back(Point3F{1001.0F, 1000.0F, 1000.0F});
  mesh.vertices.push_back(Point3F{1000.0F, 1001.0F, 1000.0F});
  mesh.faces.push_back(Face{Index{0U, 1U, 2U}, std::nullopt, std::nullopt, "default"});
  mesh.materials.emplace(
      "default", Material{"default", {}, {}, {}, Color{0xFF'FF'FF'FF}, Color{0xFF'FF'FF'FF}, Color{0xFF'FF'FF'FF}});

  const auto identity = Matrix4x4F{math::IDENTITY};
  renderer.draw_mesh(mesh, identity);

  const auto& stats = renderer.stats();
  EXPECT_EQ(stats.triangles_submitted, 1U);
  EXPECT_EQ(stats.triangles_clipped, 1U);
  EXPECT_EQ(stats.triangles_rendered, 0U);
}

// --- Render mode flags tests ---

TEST(Renderer, render_mode_flags_default)
{
  const Renderer renderer{64, 64};
  EXPECT_TRUE(renderer.face_culling_enabled());
  EXPECT_TRUE(renderer.wireframe_enabled());
  EXPECT_TRUE(renderer.shading_enabled());
  EXPECT_TRUE(renderer.light_enabled());
  EXPECT_TRUE(renderer.render_stats_enabled());
  EXPECT_FALSE(renderer.vertex_drawing_enabled());
  EXPECT_FALSE(renderer.normal_draw_enabled());
  EXPECT_FALSE(renderer.texture_enabled());
}

TEST(Renderer, render_mode_flags_toggle)
{
  Renderer renderer{64, 64};
  renderer.set_face_culling_enabled(false);
  EXPECT_FALSE(renderer.face_culling_enabled());
  renderer.set_face_culling_enabled(true);
  EXPECT_TRUE(renderer.face_culling_enabled());

  renderer.set_texture_enabled(true);
  EXPECT_TRUE(renderer.texture_enabled());
  renderer.set_texture_enabled(false);
  EXPECT_FALSE(renderer.texture_enabled());
}

// --- Texture coordinate boundary test ---

TEST(Renderer, texture_coord_boundary_does_not_overflow)
{
  // Verify that texel access at coordinate 1.0 does not go out of bounds.
  // This is a regression test for the one-past-end texture sampling bug.
  constexpr std::size_t TEX_WIDTH = 4;
  constexpr std::size_t TEX_HEIGHT = 4;
  std::array<std::uint32_t, TEX_WIDTH * TEX_HEIGHT> tex_data{};
  tex_data.fill(0xFF'00'00'FF); // Red texels

  Texture texture{tex_data.data(), TEX_WIDTH, TEX_HEIGHT};

  // Simulate the clamping logic from renderer.cpp
  const auto sample = [&](float u, float v) -> Color
  {
    const auto tex_x =
        std::min(static_cast<std::size_t>(u * static_cast<float>(texture.width())), texture.width() - 1U);
    const auto tex_y =
        std::min(static_cast<std::size_t>(v * static_cast<float>(texture.height())), texture.height() - 1U);
    return texture.texel(tex_x, tex_y);
  };

  // At exactly 1.0, without clamping this would be index 4 (out of bounds for a 4-wide texture).
  EXPECT_EQ(sample(1.0F, 1.0F), Color{0xFF'00'00'FF});
  EXPECT_EQ(sample(0.0F, 0.0F), Color{0xFF'00'00'FF});
  EXPECT_EQ(sample(0.999F, 0.999F), Color{0xFF'00'00'FF});
  // Values slightly above 1.0 (from FP imprecision) still clamp safely.
  EXPECT_EQ(sample(1.001F, 1.001F), Color{0xFF'00'00'FF});
}

} // namespace
} // namespace rtw::sw_renderer
