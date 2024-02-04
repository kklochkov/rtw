#include "math/vector.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Vector, default_ctor)
{
  constexpr rtw::math::Vector2i v;
  EXPECT_THAT(v, ::testing::ElementsAre(0, 0));
}

TEST(Vector, init_list_ctor)
{
  constexpr rtw::math::Vector2i v{1, 2};
  EXPECT_THAT(v, ::testing::ElementsAre(1, 2));
}

TEST(Vector, coversion_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 1> m{1, 2};
  constexpr rtw::math::Vector2i v{m};
  EXPECT_THAT(v, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_ctor_from_vector)
{
  constexpr rtw::math::Vector3i v1{1, 2, 3};
  constexpr rtw::math::Vector4i v2{v1};
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2, 3, 0));
}

TEST(Vector, copy_ctor)
{
  constexpr rtw::math::Vector2i v1{1, 2};
  constexpr rtw::math::Vector2i v2{v1};
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_ctor)
{
  rtw::math::Vector2i v1{1, 2};
  const rtw::math::Vector2i v2{std::move(v1)};
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_assign)
{
  constexpr rtw::math::Vector2i v1{1, 2};
  rtw::math::Vector2i v2;
  v2 = v1;
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_assign)
{
  rtw::math::Vector2i v1{1, 2};
  rtw::math::Vector2i v2;
  v2 = std::move(v1);
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, as_matrix)
{
  constexpr rtw::math::Vector2i v{1, 2};
  constexpr auto m = v.as_matrix();
  EXPECT_EQ(m.rows(), 2);
  EXPECT_EQ(m.cols(), 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2));
}

TEST(Vector, operator_brackets)
{
  rtw::math::Vector2i v{1, 2};
  v[0] = 3;
  v[1] = 4;
  EXPECT_EQ(v[0], 3);
  EXPECT_EQ(v[1], 4);
}

TEST(Vector, operator_brackets_const)
{
  constexpr rtw::math::Vector2i v{1, 2};
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
}

TEST(Vector, operator_brackets_out_of_range)
{
  rtw::math::Vector2i v{1, 2};
  v[0] = 3;
  EXPECT_DEATH(v[2], "");
}

TEST(Vector, operator_brackets_const_out_of_range)
{
  constexpr rtw::math::Vector2i v{1, 2};
  EXPECT_DEATH(v[2], "");
}

TEST(Vector, operator_plus_equal)
{
  rtw::math::Vector2i v1{1, 2};
  constexpr rtw::math::Vector2i v2{3, 4};
  v1 += v2;
  EXPECT_THAT(v1, ::testing::ElementsAre(4, 6));
}

TEST(Vector, operator_minus_equal)
{
  rtw::math::Vector2i v1{1, 2};
  constexpr rtw::math::Vector2i v2{3, 4};
  v1 -= v2;
  EXPECT_THAT(v1, ::testing::ElementsAre(-2, -2));
}

TEST(Vector, operator_multiply_equal)
{
  rtw::math::Vector2i v{1, 2};
  v *= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(2, 4));
}

TEST(Vector, operator_divide_equal)
{
  rtw::math::Vector2i v{1, 2};
  v /= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(0, 1));
}

TEST(Vector, operator_stream)
{
  constexpr rtw::math::Vector2i v{1, 2};
  std::stringstream ss;
  ss << v;
  constexpr auto expected = R"(Vector2[1 2])";
  EXPECT_EQ(ss.str(), expected);
}

TEST(Vector, accessors)
{
  {
    rtw::math::Vector3i v{1, 2, 3};
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
    rtw::math::Vector4i v{1, 2, 3, 4};
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

TEST(Vector, accessors_const)
{
  {
    constexpr rtw::math::Vector3i v{1, 2, 3};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 3);
  }
  {
    constexpr rtw::math::Vector4i v{1, 2, 3, 4};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 4);
  }
}

TEST(Vector, norm2)
{
  constexpr rtw::math::Vector3f v{1.0F, 2.0F, 3.0F};
  constexpr auto norm2 = rtw::math::norm2(v);
  EXPECT_FLOAT_EQ(norm2, 14.0F);
}

TEST(Vector, norm)
{
  constexpr rtw::math::Vector3f v{1.0F, 2.0F, 3.0F};
  EXPECT_FLOAT_EQ(rtw::math::norm(v), std::sqrt(14.0F));
}

TEST(Vector, dot)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3f v2{4.0F, 5.0F, 6.0F};
  constexpr auto dot = rtw::math::dot(v1, v2);
  EXPECT_FLOAT_EQ(dot, 32.0F);
}

TEST(Vector, cross2d)
{
  constexpr rtw::math::Vector2f v1{1.0F, 2.0F};
  constexpr rtw::math::Vector2f v2{3.0F, 4.0F};
  constexpr auto cross = rtw::math::cross(v1, v2);
  EXPECT_FLOAT_EQ(cross, -2.0F);
}

TEST(Vector, cross3d)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3f v2{4.0F, 5.0F, 6.0F};
  constexpr auto v3 = rtw::math::cross(v1, v2);
  EXPECT_THAT(v3, ::testing::ElementsAre(-3.0F, 6.0F, -3.0F));
}

TEST(Vector, operator_plus)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3f v2{4.0F, 5.0F, 6.0F};
  const auto v3 = v1 + v2;
  EXPECT_THAT(v3, ::testing::ElementsAre(5.0F, 7.0F, 9.0F));
}

TEST(Vector, operator_minus)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3f v2{4.0F, 5.0F, 6.0F};
  const auto v3 = v1 - v2;
  EXPECT_THAT(v3, ::testing::ElementsAre(-3.0F, -3.0F, -3.0F));
}

TEST(Vector, operator_multiply)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  const auto v2 = v1 * 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
  const auto v3 = 2.0F * v1;
  EXPECT_THAT(v3, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
}

TEST(Vector, operator_divide)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  const auto v2 = v1 / 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(0.5f, 1.0F, 1.5f));
}

TEST(Vector, operator_equal)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3f v2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 != v2);
}

TEST(Vector, operator_unary_minus)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  const auto v2 = -v1;
  EXPECT_THAT(v2, ::testing::ElementsAre(-1.0F, -2.0F, -3.0F));
}

TEST(Vector, normalize)
{
  constexpr rtw::math::Vector3f v1{1.0F, 2.0F, 3.0F};
  const auto v2 = rtw::math::normalize(v1);
  EXPECT_THAT(v2, ::testing::ElementsAre(1.0F / std::sqrt(14.0F), 2.0F / std::sqrt(14.0F), 3.0F / std::sqrt(14.0F)));
}

TEST(Vector, swizzle)
{
  {
    constexpr rtw::math::Vector4f v1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto v2 = v1.xy();
    EXPECT_THAT(v2, ::testing::ElementsAre(1.0F, 2.0F));
  }
  {
    constexpr rtw::math::Vector4f v1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto v2 = v1.xyz();
    EXPECT_THAT(v2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F));
  }
}
