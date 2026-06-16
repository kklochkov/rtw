#include "sw_renderer/raster_common.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{
namespace
{

TEST(RasterCommon, draw_line_dda_horizontal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 5};
  const Point2I p1{10, 5};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.y(), 5);
  }
}

TEST(RasterCommon, draw_line_dda_vertical)
{
  std::vector<Point2I> pixels;
  const Point2I p0{5, 0};
  const Point2I p1{5, 10};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.x(), 5);
  }
}

TEST(RasterCommon, draw_line_dda_diagonal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 0};
  const Point2I p1{10, 10};

  draw_line_dda(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  EXPECT_EQ(pixels.front().x(), p0.x());
  EXPECT_EQ(pixels.front().y(), p0.y());
  EXPECT_EQ(pixels.back().x(), p1.x());
  EXPECT_EQ(pixels.back().y(), p1.y());
}

TEST(RasterCommon, draw_line_bresenham_horizontal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 5};
  const Point2I p1{10, 5};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.y(), 5);
  }
}

TEST(RasterCommon, draw_line_bresenham_vertical)
{
  std::vector<Point2I> pixels;
  const Point2I p0{5, 0};
  const Point2I p1{5, 10};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  for (const auto& p : pixels)
  {
    EXPECT_EQ(p.x(), 5);
  }
}

TEST(RasterCommon, draw_line_bresenham_diagonal)
{
  std::vector<Point2I> pixels;
  const Point2I p0{0, 0};
  const Point2I p1{10, 10};

  draw_line_bresenham(p0, p1, [&pixels](const Point2I& p) { pixels.push_back(p); });

  EXPECT_EQ(pixels.size(), 11U);
  EXPECT_EQ(pixels.front().x(), p0.x());
  EXPECT_EQ(pixels.front().y(), p0.y());
  EXPECT_EQ(pixels.back().x(), p1.x());
  EXPECT_EQ(pixels.back().y(), p1.y());
}

TEST(RasterCommon, draw_line_bresenham_matches_dda_count)
{
  std::vector<Point2I> dda_pixels;
  std::vector<Point2I> bresenham_pixels;
  const Point2I p0{3, 7};
  const Point2I p1{15, 22};

  draw_line_dda(p0, p1, [&dda_pixels](const Point2I& p) { dda_pixels.push_back(p); });
  draw_line_bresenham(p0, p1, [&bresenham_pixels](const Point2I& p) { bresenham_pixels.push_back(p); });

  const auto size_diff =
      std::abs(static_cast<std::int32_t>(dda_pixels.size()) - static_cast<std::int32_t>(bresenham_pixels.size()));
  EXPECT_LE(size_diff, 1);
}

TEST(RasterCommon, is_top_left_top_edge)
{
  const math::Vector2F top_edge{-1.0F, 0.0F};
  EXPECT_TRUE(is_top_left(top_edge));
}

TEST(RasterCommon, is_top_left_left_edge)
{
  const math::Vector2F left_edge{0.5F, 1.0F};
  EXPECT_TRUE(is_top_left(left_edge));
}

TEST(RasterCommon, is_top_left_right_edge)
{
  const math::Vector2F right_edge{0.5F, -1.0F};
  EXPECT_FALSE(is_top_left(right_edge));
}

TEST(RasterCommon, is_top_left_bottom_edge)
{
  const math::Vector2F bottom_edge{1.0F, 0.0F};
  EXPECT_FALSE(is_top_left(bottom_edge));
}

} // namespace
} // namespace rtw::sw_renderer
