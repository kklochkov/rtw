#include "sw_renderer/tex_coord.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TexCoord, default_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f T;
  EXPECT_THAT(T, ::testing::ElementsAre(0, 0));
}

TEST(TexCoord, init_list_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  EXPECT_THAT(T, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<float, 2, 1> M{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f T{M};
  EXPECT_THAT(T, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector2f V{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f T{V};
  EXPECT_THAT(T, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, copy_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f T1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f T2{T1};
  EXPECT_THAT(T2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, move_ctor)
{
  rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::sw_renderer::TexCoord2f t2{std::move(t1)};
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, copy_assign)
{
  constexpr rtw::sw_renderer::TexCoord2f T1{1.0F, 2.0F};
  rtw::sw_renderer::TexCoord2f t2;
  t2 = T1;
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, move_assign)
{
  rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  rtw::sw_renderer::TexCoord2f t2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  t2 = std::move(t1);
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, operator_brackets)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  t[0] = 3.0F;
  t[1] = 4.0F;
  EXPECT_EQ(t[0], 3);
  EXPECT_EQ(t[1], 4);
}

TEST(TexCoord, operator_brackets_const)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  EXPECT_EQ(T[0], 1);
  EXPECT_EQ(T[1], 2);
}

TEST(TexCoord, operator_brackets_out_of_range)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  t[0] = 3.0F;
  EXPECT_DEATH(t[2], "");
}

TEST(TexCoord, operator_brackets_const_out_of_range)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  EXPECT_DEATH(T[2], "");
}

TEST(TexCoord, accessors)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  EXPECT_EQ(t.u(), 1);
  EXPECT_EQ(t.v(), 2);

  t.u() = 4.0F;
  t.v() = 5.0F;
  EXPECT_EQ(t.u(), 4);
  EXPECT_EQ(t.v(), 5);
}

TEST(TexCoord, accessors_const)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  EXPECT_EQ(T.u(), 1);
  EXPECT_EQ(T.v(), 2);
}

TEST(TexCoord, operator_cast_to_vector)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  constexpr auto V = static_cast<rtw::math::Vector2f>(T);
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, as_matrix)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  constexpr auto M = T.as_matrix();
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, operator_equal)
{
  constexpr rtw::sw_renderer::TexCoord2f T1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f T2{1.0F, 2.0F};
  EXPECT_TRUE(T1 == T2);
  EXPECT_FALSE(T1 != T2);
}

TEST(TextCoord, interpolation)
{
  constexpr rtw::sw_renderer::TexCoord2f T1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f T2{3.0F, 4.0F};
  const auto t = rtw::sw_renderer::lerp(T1, T2, 0.5F);
  EXPECT_THAT(t, ::testing::ElementsAre(2, 3));
}

TEST(TexCoord, operator_multiply_equal)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  t *= 2.0F;
  EXPECT_THAT(t, ::testing::ElementsAre(2, 4));
}

TEST(TexCoord, operator_divide_equal)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  t /= 2.0F;
  EXPECT_THAT(t, ::testing::ElementsAre(0.5F, 1));
}

TEST(TextCoord, operator_stream)
{
  constexpr rtw::sw_renderer::TexCoord2f T{1.0F, 2.0F};
  std::stringstream ss;
  ss << T;
  constexpr auto EXPECTED = R"(TexCoord2[1.0000 2.0000])";
  EXPECT_EQ(ss.str(), EXPECTED);
}
