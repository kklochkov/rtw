#include "math/point.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Point, default_ctor)
{
  constexpr rtw::math::Point2i P;
  EXPECT_THAT(P, ::testing::ElementsAre(0, 0));
}

TEST(Point, init_list_ctor)
{
  constexpr rtw::math::Point2i P{1, 2};
  EXPECT_THAT(P, ::testing::ElementsAre(1, 2));
}

TEST(Point, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<int, 2, 1> M{1, 2};
  constexpr rtw::math::Point2i P{M};
  EXPECT_THAT(P, ::testing::ElementsAre(1, 2));
}

TEST(Point, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector2i V{1, 2};
  constexpr rtw::math::Point2i P{V};
  EXPECT_THAT(P, ::testing::ElementsAre(1, 2));
}

TEST(Point, conversion_ctor_from_point)
{
  constexpr rtw::math::Point3i P1{1, 2, 3};
  constexpr rtw::math::Point4i P2{P1};
  EXPECT_THAT(P2, ::testing::ElementsAre(1, 2, 3, 1));
}

TEST(Point, copy_ctor)
{
  constexpr rtw::math::Point2i P1{1, 2};
  constexpr rtw::math::Point2i P2{P1};
  EXPECT_THAT(P2, ::testing::ElementsAre(1, 2));
}

TEST(Point, move_ctor)
{
  rtw::math::Point2i p1{1, 2};
  const rtw::math::Point2i p2{std::move(p1)}; // NOLINT (performance-move-const-arg) -- this is a test
  EXPECT_THAT(p2, ::testing::ElementsAre(1, 2));
}

TEST(Point, copy_assign)
{
  constexpr rtw::math::Point2i P1{1, 2};
  rtw::math::Point2i p2;
  p2 = P1;
  EXPECT_THAT(p2, ::testing::ElementsAre(1, 2));
}

TEST(Point, move_assign)
{
  rtw::math::Point2i p1{1, 2};
  rtw::math::Point2i p2;
  p2 = std::move(p1); // NOLINT (performance-move-const-arg) -- this is a test
  EXPECT_THAT(p2, ::testing::ElementsAre(1, 2));
}

TEST(Point, operator_brackets)
{
  rtw::math::Point2i p{1, 2};
  p[0] = 3;
  p[1] = 4;
  EXPECT_EQ(p[0], 3);
  EXPECT_EQ(p[1], 4);
}

TEST(Point, operator_brackets_const)
{
  constexpr rtw::math::Point2i P{1, 2};
  EXPECT_EQ(P[0], 1);
  EXPECT_EQ(P[1], 2);
}

TEST(Point, operator_brackets_out_of_range)
{
  rtw::math::Point2i p{1, 2};
  p[0] = 3;
  EXPECT_DEATH(p[2], "");
}

TEST(Point, operator_brackets_const_out_of_range)
{
  constexpr rtw::math::Point2i P{1, 2};
  EXPECT_DEATH(P[2], "");
}

TEST(Point, operator_plus_equal)
{
  rtw::math::Point2i p{1, 2};
  constexpr rtw::math::Vector2i V{3, 4};
  p += V;
  EXPECT_THAT(p, ::testing::ElementsAre(4, 6));
}

TEST(Point, operator_minus_equal)
{
  rtw::math::Point2i p{1, 2};
  constexpr rtw::math::Vector2i V{3, 4};
  p -= V;
  EXPECT_THAT(p, ::testing::ElementsAre(-2, -2));
}

TEST(Point, operator_multiply_equal)
{
  rtw::math::Point2i p{1, 2};
  p *= 2;
  EXPECT_THAT(p, ::testing::ElementsAre(2, 4));
}

TEST(Point, operator_divide_equal)
{
  rtw::math::Point2i p{1, 2};
  p /= 2;
  EXPECT_THAT(p, ::testing::ElementsAre(0, 1));
}

TEST(Point, operator_stream)
{
  constexpr rtw::math::Point2i P{1, 2};
  std::stringstream ss;
  ss << P;
  constexpr auto EXPECTED = R"(Point2[1 2])";
  EXPECT_EQ(ss.str(), EXPECTED);
}

TEST(Point, accessors)
{
  {
    rtw::math::Point3i v{1, 2, 3};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 3);

    v.x() = 4;
    v.y() = 5;
    v.z() = 6;
    EXPECT_EQ(v.x(), 4);
    EXPECT_EQ(v.y(), 5);
    EXPECT_EQ(v.z(), 6);
    EXPECT_EQ(v.w(), 6);

    v.w() = 9;
    EXPECT_EQ(v.z(), 9);
    EXPECT_EQ(v.w(), 9);
  }
  {
    rtw::math::Point4i v{1, 2, 3, 4};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 4);

    v.x() = 5;
    v.y() = 6;
    v.z() = 7;
    v.w() = 8;
    EXPECT_EQ(v.x(), 5);
    EXPECT_EQ(v.y(), 6);
    EXPECT_EQ(v.z(), 7);
    EXPECT_EQ(v.w(), 8);

    v.w() = 11;
    EXPECT_EQ(v.w(), 11);
  }
}

TEST(Point, accessors_const)
{
  {
    constexpr rtw::math::Point3i V{1, 2, 3};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 3);
  }
  {
    constexpr rtw::math::Point4i V{1, 2, 3, 4};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 4);
  }
}

TEST(Point, operator_cast_to_vector)
{
  constexpr rtw::math::Point2i P{1, 2};
  constexpr auto V = static_cast<rtw::math::Vector2i>(P);
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(Point, as_matrix)
{
  constexpr rtw::math::Point2i P{1, 2};
  constexpr auto M = P.as_matrix();
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2));
}

TEST(Point, operator_plus)
{
  constexpr rtw::math::Point2i P1{1, 2};
  constexpr rtw::math::Vector2i V{3, 4};
  const rtw::math::Point2i p2 = P1 + V;
  EXPECT_THAT(p2, ::testing::ElementsAre(4, 6));
}

TEST(Point, operator_minus)
{
  {
    // Point - Vector = Point
    constexpr rtw::math::Point2i P1{1, 2};
    constexpr rtw::math::Vector2i V{3, 4};
    const rtw::math::Point2i p2 = P1 - V;
    EXPECT_THAT(p2, ::testing::ElementsAre(-2, -2));
  }

  {
    // Point - Point = Vector
    constexpr rtw::math::Point2i P1{1, 2};
    constexpr rtw::math::Point2i P2{3, 4};
    const rtw::math::Vector2i v = P1 - P2;
    EXPECT_THAT(v, ::testing::ElementsAre(-2, -2));
  }
}

TEST(Point, operator_equal)
{
  constexpr rtw::math::Point2i P1{1, 2};
  constexpr rtw::math::Point2i P2{1, 2};
  EXPECT_TRUE(P1 == P2);
  EXPECT_FALSE(P1 != P2);
}

TEST(Point, swizzle)
{
  {
    constexpr rtw::math::Point4f V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xy();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F));
  }
  {
    constexpr rtw::math::Point4f V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xyz();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F));
  }
}
