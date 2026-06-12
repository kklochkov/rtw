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

template <typename T>
class SignedFixedPointInterpolationTest : public ::testing::Test
{};
using SignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8, rtw::multiprecision::FixedPoint16,
                                               rtw::multiprecision::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointInterpolationTest, SignedFixedPointTypes, );

TYPED_TEST(SignedFixedPointInterpolationTest, lerp)
{
  using namespace rtw::math;

  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(0.0))), 0.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(1.0))), 10.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(0.5))), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(-5.0), TypeParam(5.0), TypeParam(0.5))), 0.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(100.0), TypeParam(0.25))), 25.0,
              TypeParam::RESOLUTION);
}

TYPED_TEST(SignedFixedPointInterpolationTest, lerp_vector_per_component)
{
  using namespace rtw::math;

  using Vec = Vector2<TypeParam>;
  const auto result = lerp(Vec{TypeParam(0.0), TypeParam(0.0)}, Vec{TypeParam(10.0), TypeParam(20.0)},
                           Vec{TypeParam(0.5), TypeParam(0.25)});
  EXPECT_NEAR(static_cast<double>(result[0]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(result[1]), 5.0, TypeParam::RESOLUTION);
}

template <typename T>
class UnsignedFixedPointInterpolationTest : public ::testing::Test
{};

using UnsignedFixedPointTypes = ::testing::Types<rtw::multiprecision::FixedPoint8U, rtw::multiprecision::FixedPoint16U,
                                                 rtw::multiprecision::FixedPoint32U>;
TYPED_TEST_SUITE(UnsignedFixedPointInterpolationTest, UnsignedFixedPointTypes, );
TYPED_TEST(UnsignedFixedPointInterpolationTest, lerp)
{
  using namespace rtw::math;

  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(0.0))), 0.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(1.0))), 10.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(lerp(TypeParam(0.0), TypeParam(10.0), TypeParam(0.5))), 5.0, TypeParam::RESOLUTION);
}

TYPED_TEST(UnsignedFixedPointInterpolationTest, lerp_vector_per_component)
{
  using namespace rtw::math;

  using Vec = Vector2<TypeParam>;
  const auto result = lerp(Vec{TypeParam(0.0), TypeParam(0.0)}, Vec{TypeParam(10.0), TypeParam(20.0)},
                           Vec{TypeParam(0.5), TypeParam(0.25)});
  EXPECT_NEAR(static_cast<double>(result[0]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(result[1]), 5.0, TypeParam::RESOLUTION);
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

TEST(Interpolation, vectors_per_component)
{
  using namespace rtw::math;

  const Vector3F expected{5.0F, 5.0F, 30.0F};
  EXPECT_EQ(lerp(Vector3F{0.0F, 0.0F, 0.0F}, Vector3F{10.0F, 20.0F, 40.0F}, Vector3F{0.5F, 0.25F, 0.75F}), expected);
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
