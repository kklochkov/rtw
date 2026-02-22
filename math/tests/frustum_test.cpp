#include "math/frustum.h"
#include "math/vector_operations.h"

#include <gtest/gtest.h>

#include <cmath>

constexpr float EPSILON = 1e-5F;

TEST(Frustum, make_perspective_parameters_90_degree_fov)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);

  // For 90 degree FOV, tan(45 deg) = 1, so top = near * 1 = 0.1
  EXPECT_NEAR(params.top, 0.1F, EPSILON);
  EXPECT_NEAR(params.bottom, -0.1F, EPSILON);
  // With aspect ratio 1:1, left = top * aspect = 0.1
  EXPECT_NEAR(params.left, 0.1F, EPSILON);
  EXPECT_NEAR(params.right, -0.1F, EPSILON);
  EXPECT_FLOAT_EQ(params.near, 0.1F);
  EXPECT_FLOAT_EQ(params.far, 100.0F);
}

TEST(Frustum, make_perspective_parameters_60_degree_fov)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(60.0_degF, 1.0F, 1.0F, 100.0F);

  // For 60 degree FOV, tan(30 deg) = 1/sqrt(3) ≈ 0.577
  const float expected_top = 1.0F * std::tan(30.0F * rtw::math_constants::DEG_TO_RAD<float>);
  EXPECT_NEAR(params.top, expected_top, EPSILON);
  EXPECT_NEAR(params.bottom, -expected_top, EPSILON);
}

TEST(Frustum, make_perspective_parameters_16_9_aspect_ratio)
{
  using namespace rtw::math::angle_literals;
  const float aspect = 16.0F / 9.0F;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, aspect, 0.1F, 100.0F);

  // top = near * tan(45) = 0.1
  // left = top * aspect = 0.1 * (16/9)
  const float expected_top = 0.1F;
  const float expected_left = expected_top * aspect;
  EXPECT_NEAR(params.top, expected_top, EPSILON);
  EXPECT_NEAR(params.left, expected_left, EPSILON);
}

TEST(Frustum, make_perspective_parameters_symmetry)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);

  // Symmetric frustum: left = -right, top = -bottom
  EXPECT_FLOAT_EQ(params.left, -params.right);
  EXPECT_FLOAT_EQ(params.top, -params.bottom);
}

TEST(Frustum, perspective_projection)
{
  using namespace rtw::math::angle_literals;
  const auto frustum_params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto projection = rtw::math::make_perspective_projection_matrix(frustum_params);
  // clang-format off
  const auto expected_projection = rtw::math::Matrix4x4F{
      1.0F, 0.0F,       0.0F,        0.0F,
      0.0F, 1.0F,       0.0F,        0.0F,
      0.0F, 0.0F, -1.002002F, -0.2002002F,
      0.0F, 0.0F,      -1.0F,        0.0F,
  };
  // clang-format on
  EXPECT_EQ(projection, expected_projection);
}

TEST(Frustum, make_frustum_near_far_planes)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto frustum = rtw::math::make_frustum(params);

  // Near plane should point towards -Z (into the frustum)
  EXPECT_NEAR(frustum.near.normal.x(), 0.0F, EPSILON);
  EXPECT_NEAR(frustum.near.normal.y(), 0.0F, EPSILON);
  EXPECT_NEAR(frustum.near.normal.z(), -1.0F, EPSILON);
  EXPECT_FLOAT_EQ(frustum.near.distance, 0.1F);

  // Far plane should point towards +Z
  EXPECT_NEAR(frustum.far.normal.x(), 0.0F, EPSILON);
  EXPECT_NEAR(frustum.far.normal.y(), 0.0F, EPSILON);
  EXPECT_NEAR(frustum.far.normal.z(), 1.0F, EPSILON);
  EXPECT_FLOAT_EQ(frustum.far.distance, 100.0F);
}

TEST(Frustum, make_frustum_side_planes_are_normalized)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto frustum = rtw::math::make_frustum(params);

  // All plane normals should be unit vectors (norm 1)
  EXPECT_NEAR(rtw::math::norm(frustum.left.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.right.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.top.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.bottom.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.near.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.far.normal), 1.0F, EPSILON);
}

TEST(Frustum, make_frustum_side_planes_pass_through_origin)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto frustum = rtw::math::make_frustum(params);

  // Side planes (left, right, top, bottom) pass through the origin
  EXPECT_FLOAT_EQ(frustum.left.distance, 0.0F);
  EXPECT_FLOAT_EQ(frustum.right.distance, 0.0F);
  EXPECT_FLOAT_EQ(frustum.top.distance, 0.0F);
  EXPECT_FLOAT_EQ(frustum.bottom.distance, 0.0F);
}

TEST(Frustum, extract_frustum_plane_normals_are_normalized)
{
  using namespace rtw::math::angle_literals;
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto projection = rtw::math::make_perspective_projection_matrix(params);
  const auto frustum = rtw::math::extract_frustum(projection);

  // All plane normals should be unit vectors
  EXPECT_NEAR(rtw::math::norm(frustum.left.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.right.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.top.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.bottom.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.near.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.far.normal), 1.0F, EPSILON);
}

TEST(Frustum, extract_frustum_from_identity_matrix)
{
  const rtw::math::Matrix4x4F identity{rtw::math::IDENTITY};
  const auto frustum = rtw::math::extract_frustum(identity);

  // For identity matrix, the frustum should be a unit cube [-1, 1]^3
  // All plane normals should be unit vectors
  EXPECT_NEAR(rtw::math::norm(frustum.left.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.right.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.top.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.bottom.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.near.normal), 1.0F, EPSILON);
  EXPECT_NEAR(rtw::math::norm(frustum.far.normal), 1.0F, EPSILON);
}

TEST(Frustum, extract_frustum_from_column_major_matrix)
{
  using namespace rtw::math::angle_literals;

  // Create perspective projection with ROW_MAJOR (default)
  const auto params = rtw::math::make_perspective_parameters(90.0_degF, 1.0F, 0.1F, 100.0F);
  const auto projection_row_major = rtw::math::make_perspective_projection_matrix(params);

  // Convert to COLUMN_MAJOR
  const auto projection_col_major = projection_row_major.cast<rtw::math::MemoryOrder::COLUMN_MAJOR>();

  // Extract frustum from both - should produce identical results
  const auto frustum_rm = rtw::math::extract_frustum(projection_row_major);
  const auto frustum_cm = rtw::math::extract_frustum(projection_col_major);

  // Verify plane normals match between ROW_MAJOR and COLUMN_MAJOR extraction
  EXPECT_NEAR(frustum_cm.left.normal.x(), frustum_rm.left.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.left.normal.y(), frustum_rm.left.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.left.normal.z(), frustum_rm.left.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.left.distance, frustum_rm.left.distance, EPSILON);

  EXPECT_NEAR(frustum_cm.right.normal.x(), frustum_rm.right.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.right.normal.y(), frustum_rm.right.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.right.normal.z(), frustum_rm.right.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.right.distance, frustum_rm.right.distance, EPSILON);

  EXPECT_NEAR(frustum_cm.near.normal.x(), frustum_rm.near.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.near.normal.y(), frustum_rm.near.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.near.normal.z(), frustum_rm.near.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.near.distance, frustum_rm.near.distance, EPSILON);

  EXPECT_NEAR(frustum_cm.far.normal.x(), frustum_rm.far.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.far.normal.y(), frustum_rm.far.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.far.normal.z(), frustum_rm.far.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.far.distance, frustum_rm.far.distance, EPSILON);

  EXPECT_NEAR(frustum_cm.top.normal.x(), frustum_rm.top.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.top.normal.y(), frustum_rm.top.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.top.normal.z(), frustum_rm.top.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.top.distance, frustum_rm.top.distance, EPSILON);

  EXPECT_NEAR(frustum_cm.bottom.normal.x(), frustum_rm.bottom.normal.x(), EPSILON);
  EXPECT_NEAR(frustum_cm.bottom.normal.y(), frustum_rm.bottom.normal.y(), EPSILON);
  EXPECT_NEAR(frustum_cm.bottom.normal.z(), frustum_rm.bottom.normal.z(), EPSILON);
  EXPECT_NEAR(frustum_cm.bottom.distance, frustum_rm.bottom.distance, EPSILON);
}

TEST(Frustum, make_perspective_parameters_and_projection_roundtrip)
{
  using namespace rtw::math::angle_literals;

  // Create multiple frustums with different parameters and verify consistency
  const std::vector<std::tuple<rtw::math::AngleF, float, float, float>> test_cases = {
      {90.0_degF, 1.0F, 0.1F, 100.0F},
      {60.0_degF, 16.0F / 9.0F, 0.5F, 500.0F},
      {45.0_degF, 4.0F / 3.0F, 1.0F, 1000.0F},
  };

  for (const auto& [fov, aspect, near_plane, far_plane] : test_cases)
  {
    const auto params = rtw::math::make_perspective_parameters(fov, aspect, near_plane, far_plane);
    const auto projection = rtw::math::make_perspective_projection_matrix(params);

    // Verify near/far are preserved
    EXPECT_FLOAT_EQ(params.near, near_plane);
    EXPECT_FLOAT_EQ(params.far, far_plane);

    // Verify projection matrix has correct structure
    EXPECT_FLOAT_EQ(projection(3, 2), -1.0F); // Perspective divide
    EXPECT_FLOAT_EQ(projection(3, 3), 0.0F);
  }
}
