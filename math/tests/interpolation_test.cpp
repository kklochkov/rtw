#include "math/format.h" // IWYU pragma: keep
#include "math/interpolation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Interpolation, floats)
{
  using namespace rtw::math;

  EXPECT_EQ(lerp(0.0F, 1.0F, 0.0F), 0.0F);
  EXPECT_EQ(lerp(0.0F, 1.0F, 0.5F), 0.5F);
  EXPECT_EQ(lerp(0.0F, 1.0F, 1.0F), 1.0F);

  EXPECT_EQ(lerp(0.0F, 1.0F, 0.0F), 0.0F);
  EXPECT_EQ(lerp(0.0F, 1.0F, 0.5F), 0.5F);
  EXPECT_EQ(lerp(0.0F, 1.0F, 1.0F), 1.0F);
}

TEST(Interpolation, vectors)
{
  using namespace rtw::math;

  Vector2F expected{0.0F, 0.0F};
  EXPECT_EQ(lerp(Vector2F{0.0F, 0.0F}, Vector2F{1.0F, 1.0F}, 0.0F), expected);

  expected = Vector2F{0.5F, 0.5F};
  EXPECT_EQ(lerp(Vector2F{0.0F, 0.0F}, Vector2F{1.0F, 1.0F}, 0.5F), expected);

  expected = Vector2F{1.0F, 1.0F};
  EXPECT_EQ(lerp(Vector2F{0.0F, 0.0F}, Vector2F{1.0F, 1.0F}, 1.0F), expected);
}

TEST(Interpolation, points)
{
  using namespace rtw::math;

  Point2F expected{0.0F, 0.0F};
  EXPECT_EQ(lerp(Point2F{0.0F, 0.0F}, Point2F{1.0F, 1.0F}, 0.0F), expected);

  expected = Point2F{0.5F, 0.5F};
  EXPECT_EQ(lerp(Point2F{0.0F, 0.0F}, Point2F{1.0F, 1.0F}, 0.5F), expected);

  expected = Point2F{1.0F, 1.0F};
  EXPECT_EQ(lerp(Point2F{0.0F, 0.0F}, Point2F{1.0F, 1.0F}, 1.0F), expected);
}
