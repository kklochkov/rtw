#include "sw_renderer/projection.h"

#include <gtest/gtest.h>

TEST(Projection, perspective_projection)
{
  using namespace rtw::math::angle_literals;
  const auto frustum_params = rtw::sw_renderer::make_perspective_parameters(90.0_degf, 1.0F, 0.1F, 100.0F);
  const auto projection = rtw::sw_renderer::make_perspective_projection_matrix(frustum_params);
  // clang-format off
  const auto expected_projection = rtw::math::Matrix4x4f{
      1.0F, 0.0F,       0.0F,        0.0F,
      0.0F, 1.0F,       0.0F,        0.0F,
      0.0F, 0.0F, -1.002002F, -0.2002002F,
      0.0F, 0.0F,      -1.0F,        0.0F,
  };
  // clang-format on
  EXPECT_EQ(projection, expected_projection);
}
