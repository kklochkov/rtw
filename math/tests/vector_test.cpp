#include "math/format.h" // IWYU pragma: keep
#include "math/vector.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Vector, default_ctor)
{
  constexpr rtw::math::Vector2I V;
  EXPECT_THAT(V, ::testing::ElementsAre(0, 0));
}

TEST(Vector, init_list_ctor)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(Vector, coversion_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 1> M{1, 2};
  constexpr rtw::math::Vector2I V{M};
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_ctor_from_vector)
{
  constexpr rtw::math::Vector3I V1{1, 2, 3};
  constexpr rtw::math::Vector4I V2{V1};
  EXPECT_THAT(V2, ::testing::ElementsAre(1, 2, 3, 0));
}

TEST(Vector, copy_ctor)
{
  constexpr rtw::math::Vector2I V1{1, 2};
  constexpr rtw::math::Vector2I V2{V1};
  EXPECT_THAT(V2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_ctor)
{
  rtw::math::Vector2I v1{1, 2};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::math::Vector2I v2{std::move(v1)};
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_assign)
{
  constexpr rtw::math::Vector2I V1{1, 2};
  rtw::math::Vector2I v2;
  v2 = V1;
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_assign)
{
  rtw::math::Vector2I v1{1, 2};
  rtw::math::Vector2I v2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  v2 = std::move(v1);
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, as_matrix)
{
  constexpr rtw::math::Vector2I V{1, 2};
  constexpr auto M = V.as_matrix();
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2));
}

TEST(Vector, operator_brackets)
{
  rtw::math::Vector2I v{1, 2};
  v[0] = 3;
  v[1] = 4;
  EXPECT_EQ(v[0], 3);
  EXPECT_EQ(v[1], 4);
}

TEST(Vector, operator_brackets_const)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_EQ(V[0], 1);
  EXPECT_EQ(V[1], 2);
}

TEST(Vector, operator_brackets_out_of_range)
{
  rtw::math::Vector2I v{1, 2};
  v[0] = 3;
  EXPECT_DEATH(v[2], "");
}

TEST(Vector, operator_brackets_const_out_of_range)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_DEATH(V[2], "");
}

TEST(Vector, operator_plus_equal)
{
  rtw::math::Vector2I v1{1, 2};
  constexpr rtw::math::Vector2I V2{3, 4};
  v1 += V2;
  EXPECT_THAT(v1, ::testing::ElementsAre(4, 6));
}

TEST(Vector, operator_minus_equal)
{
  rtw::math::Vector2I v1{1, 2};
  constexpr rtw::math::Vector2I V2{3, 4};
  v1 -= V2;
  EXPECT_THAT(v1, ::testing::ElementsAre(-2, -2));
}

TEST(Vector, operator_multiply_equal)
{
  rtw::math::Vector2I v{1, 2};
  v *= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(2, 4));
}

TEST(Vector, operator_divide_equal)
{
  rtw::math::Vector2I v{1, 2};
  v /= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(0, 1));
}

TEST(Vector, operator_stream)
{
  constexpr rtw::math::Vector2I V{1, 2};
  std::stringstream ss;
  ss << V;
  constexpr auto EXPECTED = R"(Vector2[1 2])";
  EXPECT_EQ(ss.str(), EXPECTED);
}

TEST(Vector, accessors)
{
  {
    rtw::math::Vector3I v{1, 2, 3};
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
    rtw::math::Vector4I v{1, 2, 3, 4};
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
    constexpr rtw::math::Vector3I V{1, 2, 3};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 3);
  }
  {
    constexpr rtw::math::Vector4I V{1, 2, 3, 4};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 4);
  }
}

TEST(Vector, norm2)
{
  constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
  constexpr auto NORM2 = rtw::math::norm2(V);
  EXPECT_FLOAT_EQ(NORM2, 14.0F);
}

TEST(Vector, norm)
{
  constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
  EXPECT_FLOAT_EQ(rtw::math::norm(V), std::sqrt(14.0F));
}

TEST(Vector, dot)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  constexpr auto DOT = rtw::math::dot(V1, V2);
  EXPECT_FLOAT_EQ(DOT, 32.0F);
}

TEST(Vector, cross2d)
{
  constexpr rtw::math::Vector2F V1{1.0F, 2.0F};
  constexpr rtw::math::Vector2F V2{3.0F, 4.0F};
  constexpr auto CROSS = rtw::math::cross(V1, V2);
  EXPECT_FLOAT_EQ(CROSS, -2.0F);
}

TEST(Vector, cross3d)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  constexpr auto V3 = rtw::math::cross(V1, V2);
  EXPECT_THAT(V3, ::testing::ElementsAre(-3.0F, 6.0F, -3.0F));
}

TEST(Vector, operator_plus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  const auto v3 = V1 + V2;
  EXPECT_THAT(v3, ::testing::ElementsAre(5.0F, 7.0F, 9.0F));
}

TEST(Vector, operator_minus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  const auto v3 = V1 - V2;
  EXPECT_THAT(v3, ::testing::ElementsAre(-3.0F, -3.0F, -3.0F));
}

TEST(Vector, operator_multiply)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = V1 * 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
  const auto v3 = 2.0F * V1;
  EXPECT_THAT(v3, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
}

TEST(Vector, operator_divide)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = V1 / 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(0.5F, 1.0F, 1.5F));
}

TEST(Vector, operator_equal)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(V1 == V2);
  EXPECT_FALSE(V1 != V2);
}

TEST(Vector, operator_unary_minus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = -V1;
  EXPECT_THAT(v2, ::testing::ElementsAre(-1.0F, -2.0F, -3.0F));
}

TEST(Vector, normalize)
{
  {
    constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
    const auto v2 = rtw::math::normalize(V1);
    EXPECT_THAT(v2, ::testing::ElementsAre(1.0F / std::sqrt(14.0F), 2.0F / std::sqrt(14.0F), 3.0F / std::sqrt(14.0F)));
  }
  {
    EXPECT_DEATH(rtw::math::normalize(rtw::math::Vector3F{0.0F, 0.0F, 0.0F}), "");
  }
}

TEST(Vector, swizzle)
{
  {
    constexpr rtw::math::Vector4F V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xy();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F));
  }
  {
    constexpr rtw::math::Vector4F V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xyz();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F));
  }
}
