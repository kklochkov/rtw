#include "sw_renderer/clip_space.h"
#include "sw_renderer/clipping.h"
#include "sw_renderer/types.h"

#include "math/angle.h"
#include "math/frustum.h"
#include "math/plane.h"
#include "sw_renderer/vertex.h"

#include <gtest/gtest.h>

namespace rtw::sw_renderer
{
namespace
{

template <typename T, std::size_t CAPACITY = 9U>
using ConvexPolygonVertex = math::ConvexPolygon<T, Vertex, CAPACITY>;

// Helper to create a vertex at a given position
VertexF make_vertex(const float x, const float y, const float z)
{
  return VertexF{math::Point4<single_precision>{x, y, z, 1.0F}};
}
// Helper to create a clip-space vertex with a single scalar varying in slot 0.
ClipVertex<single_precision> make_clip_vertex(const float x, const float y, const float z, const float w,
                                              const float varying = 0.0F)
{
  ClipVertex<single_precision> vertex;
  vertex.position = math::Vector4<single_precision>{x, y, z, w};
  vertex.varyings[0] = math::Vector4<single_precision>{varying, 0.0F, 0.0F, 0.0F};
  return vertex;
}

// Create a test frustum with known parameters
math::Frustum3F make_test_frustum()
{
  using namespace rtw::math;
  const auto fov_y = Angle<float>{DEG, 60.0F};
  constexpr auto ASPECT_RATIO = 4.0F / 3.0F;
  constexpr auto NEAR = 0.1F;
  constexpr auto FAR = 100.0F;

  const auto params = make_perspective_parameters(fov_y, ASPECT_RATIO, NEAR, FAR);
  return make_frustum(params);
}

TEST(Clipping, clip_triangle_inside_frustum)
{
  // Create a frustum for testing
  const auto frustum = make_test_frustum();

  // Triangle well inside the frustum (using negative Z, as camera looks down -Z)
  // Near plane is at z = -0.1, far plane at z = -100
  // Triangle at z = -5 should be inside
  const auto v0 = make_vertex(0.0F, 0.0F, -5.0F);
  const auto v1 = make_vertex(0.5F, 0.0F, -5.0F);
  const auto v2 = make_vertex(0.0F, 0.5F, -5.0F);

  const auto result = clip(v0, v1, v2, stl::make_span(frustum.planes()));

  // Triangle fully inside, should have 3 vertices
  EXPECT_EQ(result.size(), 3U);
}

TEST(Clipping, clip_triangle_outside_frustum_behind_camera)
{
  const auto frustum = make_test_frustum();

  // Triangle behind the camera (positive Z, in front of camera position at origin)
  const auto v0 = make_vertex(0.0F, 0.0F, 1.0F);
  const auto v1 = make_vertex(0.5F, 0.0F, 1.0F);
  const auto v2 = make_vertex(0.0F, 0.5F, 1.0F);

  const auto result = clip(v0, v1, v2, stl::make_span(frustum.planes()));

  // Triangle fully outside (behind camera), should be empty
  EXPECT_EQ(result.size(), 0U);
}

TEST(Clipping, clip_triangle_outside_frustum_beyond_far)
{
  const auto frustum = make_test_frustum();

  // Triangle beyond far plane (z < -100)
  const auto v0 = make_vertex(0.0F, 0.0F, -150.0F);
  const auto v1 = make_vertex(0.5F, 0.0F, -150.0F);
  const auto v2 = make_vertex(0.0F, 0.5F, -150.0F);

  const auto result = clip(v0, v1, v2, stl::make_span(frustum.planes()));

  // Triangle fully outside (beyond far), should be empty
  EXPECT_EQ(result.size(), 0U);
}

TEST(Clipping, clip_triangle_straddling_near_plane)
{
  const auto frustum = make_test_frustum();

  // Triangle that straddles the near plane (near = 0.1)
  // One vertex behind near plane (z > -0.1), two in front (z < -0.1)
  const auto v0 = make_vertex(0.0F, 0.0F, -1.0F); // inside
  const auto v1 = make_vertex(0.5F, 0.0F, -1.0F); // inside
  const auto v2 = make_vertex(0.25F, 0.5F, 0.0F); // outside (behind near plane)

  const auto result = clip(v0, v1, v2, stl::make_span(frustum.planes()));

  // One vertex clipped by near plane creates intersection points
  // The result should have more than 2 vertices (at least a triangle)
  // and less than the original + max intersection points
  EXPECT_GE(result.size(), 3U);
  EXPECT_LE(result.size(), 6U);
}

TEST(Clipping, triangulate_quad)
{
  // Create a quad (4 vertices) and triangulate it
  const auto v0 = make_vertex(0.0F, 0.0F, -5.0F);
  const auto v1 = make_vertex(1.0F, 0.0F, -5.0F);
  const auto v2 = make_vertex(1.0F, 1.0F, -5.0F);
  const auto v3 = make_vertex(0.0F, 1.0F, -5.0F);

  ConvexPolygonVertex<single_precision> quad;
  quad.push_back(v0);
  quad.push_back(v1);
  quad.push_back(v2);
  quad.push_back(v3);

  const auto result = triangulate(quad);

  // 4 vertices -> 2 triangles
  EXPECT_EQ(result.triangle_count, 2U);
}

TEST(Clipping, triangulate_pentagon)
{
  // Create a pentagon (5 vertices) and triangulate it
  const auto v0 = make_vertex(0.0F, 0.0F, -5.0F);
  const auto v1 = make_vertex(1.0F, 0.0F, -5.0F);
  const auto v2 = make_vertex(1.5F, 0.5F, -5.0F);
  const auto v3 = make_vertex(0.5F, 1.0F, -5.0F);
  const auto v4 = make_vertex(-0.5F, 0.5F, -5.0F);

  ConvexPolygonVertex<single_precision> pentagon;
  pentagon.push_back(v0);
  pentagon.push_back(v1);
  pentagon.push_back(v2);
  pentagon.push_back(v3);
  pentagon.push_back(v4);

  const auto result = triangulate(pentagon);

  // 5 vertices -> 3 triangles
  EXPECT_EQ(result.triangle_count, 3U);
}

TEST(Clipping, triangulate_triangle)
{
  // Triangulating a triangle should result in 1 triangle
  const auto v0 = make_vertex(0.0F, 0.0F, -5.0F);
  const auto v1 = make_vertex(1.0F, 0.0F, -5.0F);
  const auto v2 = make_vertex(0.5F, 1.0F, -5.0F);

  ConvexPolygonVertex<single_precision> triangle;
  triangle.push_back(v0);
  triangle.push_back(v1);
  triangle.push_back(v2);

  const auto result = triangulate(triangle);

  // 3 vertices -> 1 triangle
  EXPECT_EQ(result.triangle_count, 1U);
}

TEST(Clipping, triangulate_invalid_polygon)
{
  // Polygon with less than 3 vertices is invalid
  ConvexPolygonVertex<single_precision> invalid_polygon;
  invalid_polygon.push_back(make_vertex(0.0F, 0.0F, -5.0F));
  invalid_polygon.push_back(make_vertex(1.0F, 0.0F, -5.0F));

  const auto result = triangulate(invalid_polygon);

  // Invalid polygon -> 0 triangles
  EXPECT_EQ(result.triangle_count, 0U);
}

TEST(Clipping, triangulate_empty_polygon)
{
  // Empty polygon
  ConvexPolygonVertex<single_precision> empty_polygon;

  const auto result = triangulate(empty_polygon);

  // Empty polygon -> 0 triangles
  EXPECT_EQ(result.triangle_count, 0U);
}

TEST(Clipping, clip_space_triangle_inside)
{
  // |x|, |y|, |z| <= w, so the triangle lies entirely within the clip volume.
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(0.5F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F);

  const auto result = clip(v0, v1, v2);

  // Triangle fully inside the clip volume keeps its three vertices.
  EXPECT_EQ(result.size(), 3U);
}

TEST(Clipping, clip_space_triangle_outside)
{
  // All vertices have x > w, so the whole triangle is beyond the right plane.
  const auto v0 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(3.0F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(2.0F, 1.0F, 0.0F, 1.0F);

  const auto result = clip(v0, v1, v2);

  // Triangle fully outside the clip volume is rejected.
  EXPECT_EQ(result.size(), 0U);
}

TEST(Clipping, clip_space_triangle_straddling_near)
{
  // One vertex behind the near plane (z + w < 0), two in front of it.
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F);
  const auto v1 = make_clip_vertex(0.5F, 0.0F, 0.0F, 1.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, -2.0F, 1.0F);

  const auto result = clip(v0, v1, v2);

  // Clipping the single outside vertex against the near plane turns the triangle into a quad.
  EXPECT_EQ(result.size(), 4U);
}

TEST(Clipping, clip_space_interpolates_varyings)
{
  // v1 is outside the right plane (x > w); the v0->v1 edge crosses the plane at t = 0.5.
  const auto v0 = make_clip_vertex(0.0F, 0.0F, 0.0F, 1.0F, 10.0F);
  const auto v1 = make_clip_vertex(2.0F, 0.0F, 0.0F, 1.0F, 20.0F);
  const auto v2 = make_clip_vertex(0.0F, 0.5F, 0.0F, 1.0F, 30.0F);

  const auto result = clip(v0, v1, v2);

  // Only the right plane clips, leaving [v0, A, B, v2]; A lies on x = w with the lerped varying.
  ASSERT_EQ(result.size(), 4U);
  EXPECT_FLOAT_EQ(result[1].position[0], 1.0F);
  EXPECT_FLOAT_EQ(result[1].varyings[0][0], 15.0F);
}

} // namespace
} // namespace rtw::sw_renderer
