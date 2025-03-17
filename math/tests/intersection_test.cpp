#include "math/intersection.h"

#include <gtest/gtest.h>

TEST(Intersection, intersection_plane_line_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F),
                                                               Point2F(0.0F, 1.0F), Point2F(1.0F, 1.0F)),
            std::numeric_limits<float>::max());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F),
                                                               Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DONT_CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F),
                                                                    Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            1.0F);
}

TEST(Intersection, intersection_plane_line_point)
{
  using namespace rtw::math;

  constexpr Point2F MAX{INITIALIZE_WITH_VALUE, std::numeric_limits<float>::max()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F),
                                                        Point2F(1.0F, 1.0F)),
            MAX);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F), Point2F(0.0F, 1.0F),
                                                        Point2F(1.0F, 0.0F)),
            Point2F(1.0F, 0.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DONT_CHECK>(Point2F(0.0F, 0.0F), Vector2F(0.0F, 1.0F),
                                                             Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            Point2F(1.0F, 0.0F));
}

TEST(Intersection, intersection_lines_factor)
{
  using namespace rtw::math;

  // Parallel
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F),
                                                               Point2F(1.0F, 0.0F), Point2F(1.0F, 1.0F)),
            std::numeric_limits<float>::max());

  // Intersection
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F),
                                                               Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            1.0F);
  EXPECT_EQ(intersection_factor<CalculationCheckPolicy::DONT_CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F),
                                                                    Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            1.0F);
}

TEST(Intersection, intersection_lines_point)
{
  using namespace rtw::math;

  constexpr Point2F MAX{INITIALIZE_WITH_VALUE, std::numeric_limits<float>::max()};

  // Parallel
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F),
                                                        Point2F(1.0F, 1.0F)),
            MAX);

  // Intersection
  EXPECT_EQ(intersection<CalculationCheckPolicy::CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F), Point2F(0.0F, 1.0F),
                                                        Point2F(1.0F, 0.0F)),
            Point2F(0.0F, 1.0F));
  EXPECT_EQ(intersection<CalculationCheckPolicy::DONT_CHECK>(Point2F(0.0F, 0.0F), Point2F(0.0F, 1.0F),
                                                             Point2F(0.0F, 1.0F), Point2F(1.0F, 0.0F)),
            Point2F(0.0F, 1.0F));
}
