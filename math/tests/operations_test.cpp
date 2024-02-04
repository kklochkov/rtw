#include "math/operations.h"

#include <gtest/gtest.h>

TEST(Operations, intersection_plane_line_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 1.0F)),
            std::numeric_limits<float>::infinity());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DontCheck>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                                   Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
}

TEST(Operations, intersection_plane_line_point)
{
  using namespace rtw::math;

  const Point2f infinity{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 1.0F)),
            infinity);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 0.0F)),
            Point2f(1.0F, 0.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DontCheck>(Point2f(0.0F, 0.0F), Vector2f(0.0F, 1.0F),
                                                            Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            Point2f(1.0F, 0.0F));
}

TEST(Operations, intersection_lines_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                               Point2f(1.0F, 0.0F), Point2f(1.0F, 1.0F)),
            std::numeric_limits<float>::infinity());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                               Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DontCheck>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                                   Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            1.0F);
}

TEST(Operations, intersection_lines_point)
{
  using namespace rtw::math;

  const Point2f infinity{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F),
                                                        Point2f(1.0F, 1.0F)),
            infinity);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::Check>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F), Point2f(0.0F, 1.0F),
                                                        Point2f(1.0F, 0.0F)),
            Point2f(0.0F, 1.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DontCheck>(Point2f(0.0F, 0.0F), Point2f(0.0F, 1.0F),
                                                            Point2f(0.0F, 1.0F), Point2f(1.0F, 0.0F)),
            Point2f(0.0F, 1.0F));
}
