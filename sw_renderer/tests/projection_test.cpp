#include "sw_renderer/projection.h"
#include "sw_renderer/types.h"

#include <gtest/gtest.h>

namespace rtw::sw_renderer
{
namespace
{

TEST(Projection, make_screen_space_matrix_dimensions)
{
  constexpr std::size_t WIDTH = 640;
  constexpr std::size_t HEIGHT = 480;

  const auto matrix = make_screen_space_matrix<single_precision>(WIDTH, HEIGHT);

  // sx = (width - 1) / 2 = 319.5
  // sy = -(height - 1) / 2 = -239.5
  // tx = (width - 1) / 2 = 319.5
  // ty = (height - 1) / 2 = 239.5
  constexpr auto EXPECTED_TX = (WIDTH - 1) / 2.0F;
  constexpr auto EXPECTED_TY = (HEIGHT - 1) / 2.0F;
  constexpr auto EXPECTED_SX = EXPECTED_TX;
  constexpr auto EXPECTED_SY = -EXPECTED_TY;

  EXPECT_FLOAT_EQ(matrix(0, 0), EXPECTED_SX);
  EXPECT_FLOAT_EQ(matrix(1, 1), EXPECTED_SY);
  EXPECT_FLOAT_EQ(matrix(0, 3), EXPECTED_TX);
  EXPECT_FLOAT_EQ(matrix(1, 3), EXPECTED_TY);
}

TEST(Projection, ndc_to_screen_space_center)
{
  constexpr std::size_t WIDTH = 640;
  constexpr std::size_t HEIGHT = 480;

  const auto matrix = make_screen_space_matrix<single_precision>(WIDTH, HEIGHT);
  const Point4F ndc_center{0.0F, 0.0F, 0.0F, 1.0F};

  const auto screen = ndc_to_screen_space(ndc_center, matrix);

  // Center of screen: ((width-1)/2, (height-1)/2)
  EXPECT_FLOAT_EQ(screen.x(), 319.5F);
  EXPECT_FLOAT_EQ(screen.y(), 239.5F);
}

TEST(Projection, ndc_to_screen_space_top_left)
{
  constexpr std::size_t WIDTH = 640;
  constexpr std::size_t HEIGHT = 480;

  const auto matrix = make_screen_space_matrix<single_precision>(WIDTH, HEIGHT);
  // NDC: (-1, 1) should map to screen top-left (0, 0)
  // Note: y is flipped, so NDC y=1 (top in NDC) maps to screen y=0 (top in screen)
  const Point4F ndc_top_left{-1.0F, 1.0F, 0.0F, 1.0F};

  const auto screen = ndc_to_screen_space(ndc_top_left, matrix);

  EXPECT_FLOAT_EQ(screen.x(), 0.0F);
  EXPECT_FLOAT_EQ(screen.y(), 0.0F);
}

TEST(Projection, ndc_to_screen_space_bottom_right)
{
  constexpr std::size_t WIDTH = 640;
  constexpr std::size_t HEIGHT = 480;

  const auto matrix = make_screen_space_matrix<single_precision>(WIDTH, HEIGHT);
  // NDC: (1, -1) should map to screen bottom-right (639, 479)
  const Point4F ndc_bottom_right{1.0F, -1.0F, 0.0F, 1.0F};

  const auto screen = ndc_to_screen_space(ndc_bottom_right, matrix);

  EXPECT_FLOAT_EQ(screen.x(), 639.0F);
  EXPECT_FLOAT_EQ(screen.y(), 479.0F);
}

TEST(Projection, ndc_to_screen_space_preserves_w)
{
  constexpr std::size_t WIDTH = 640;
  constexpr std::size_t HEIGHT = 480;

  const auto matrix = make_screen_space_matrix<single_precision>(WIDTH, HEIGHT);
  constexpr auto ORIGINAL_W = 2.5F;
  const Point4F ndc_point{0.5F, -0.5F, 0.3F, ORIGINAL_W};

  const auto screen = ndc_to_screen_space(ndc_point, matrix);

  // w should be preserved for depth buffer and perspective correct interpolation
  EXPECT_FLOAT_EQ(screen.w(), ORIGINAL_W);
}

} // namespace
} // namespace rtw::sw_renderer
