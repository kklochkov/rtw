#include "math/operations.h"

#include <gtest/gtest.h>

TEST(Operations, intersection_plane_line_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 1.0F)),
            std::numeric_limits<float>::max());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DONT_CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                                    Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
}

TEST(Operations, intersection_plane_line_point)
{
  using namespace rtw::math;

  const Point2f infinity{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 1.0F)),
            infinity);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 0.0F)),
            Point2f(1.0F, 0.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DONT_CHECK>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                             Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            Point2f(1.0F, 0.0F));
}

TEST(Operations, intersection_lines_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                               Point2f(1.0F, 0.0F), Point2f(1.0F, 1.0F)),
            std::numeric_limits<float>::max());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DONT_CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                                    Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
}

TEST(Operations, intersection_lines_point)
{
  using namespace rtw::math;

  const Point2f infinity{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F),
                                                        Point2f(1.0F, 1.0F)),
            infinity);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 0.0F)),
            Point2f(0.0F, 1.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DONT_CHECK>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                             Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            Point2f(0.0F, 1.0F));
}
