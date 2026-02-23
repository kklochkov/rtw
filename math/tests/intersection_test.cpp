#include "math/intersection.h"

#include <gtest/gtest.h>

TEST(Intersection, intersection_plane_line_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 1.0F)),
            std::nullopt);

  // Intersection
  EXPECT_EQ(
      intersection_factor(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
      1.0F);
  EXPECT_EQ(
      intersection_factor(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
      1.0F);
}

TEST(Intersection, intersection_plane_line_point)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 1.0F)),
            std::nullopt);

  // Intersection
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
            Point2F(1.0F, 0.0F));
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
            Point2F(1.0F, 0.0F));
}

TEST(Intersection, intersection_lines_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F), Point2F(1.0F, 1.0F)),
            std::nullopt);

  // Intersection
  EXPECT_EQ(
      intersection_factor(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
      1.0F);
  EXPECT_EQ(
      intersection_factor(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
      1.0F);
}

TEST(Intersection, intersection_lines_point)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F), Point2F(1.0F, 1.0F)),
            std::nullopt);

  // Intersection
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
            Point2F(0.0F, 1.0F));
  EXPECT_EQ(intersection(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)).value(),
            Point2F(0.0F, 1.0F));
}
