#include "sw_renderer/tex_coord.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TexCoord, default_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f t;
  EXPECT_THAT(t, ::testing::ElementsAre(0, 0));
}

TEST(TexCoord, init_list_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  EXPECT_THAT(t, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<float, 2, 1> m{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f t{m};
  EXPECT_THAT(t, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector2f v{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f t{v};
  EXPECT_THAT(t, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, copy_ctor)
{
  constexpr rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f t2{t1};
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, move_ctor)
{
  rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  const rtw::sw_renderer::TexCoord2f t2{std::move(t1)};
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, copy_assign)
{
  constexpr rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  rtw::sw_renderer::TexCoord2f t2;
  t2 = t1;
  EXPECT_THAT(t2, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, move_assign)
{
  rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  rtw::sw_renderer::TexCoord2f t2;
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
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  EXPECT_EQ(t[0], 1);
  EXPECT_EQ(t[1], 2);
}

TEST(TexCoord, operator_brackets_out_of_range)
{
  rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  t[0] = 3.0F;
  EXPECT_DEATH(t[2], "");
}

TEST(TexCoord, operator_brackets_const_out_of_range)
{
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  EXPECT_DEATH(t[2], "");
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
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  EXPECT_EQ(t.u(), 1);
  EXPECT_EQ(t.v(), 2);
}

TEST(TexCoord, operator_cast_to_vector)
{
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  constexpr auto v = static_cast<rtw::math::Vector2f>(t);
  EXPECT_THAT(v, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, as_matrix)
{
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  constexpr auto m = t.as_matrix();
  EXPECT_EQ(m.rows(), 2);
  EXPECT_EQ(m.cols(), 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2));
}

TEST(TexCoord, operator_equal)
{
  constexpr rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f t2{1.0F, 2.0F};
  EXPECT_TRUE(t1 == t2);
  EXPECT_FALSE(t1 != t2);
}

TEST(TextCoord, interpolation)
{
  constexpr rtw::sw_renderer::TexCoord2f t1{1.0F, 2.0F};
  constexpr rtw::sw_renderer::TexCoord2f t2{3.0F, 4.0F};
  const auto t = rtw::sw_renderer::lerp(t1, t2, 0.5F);
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
  constexpr rtw::sw_renderer::TexCoord2f t{1.0F, 2.0F};
  std::stringstream ss;
  ss << t;
  constexpr auto expected = R"(TexCoord2[1.0000 2.0000])";
  EXPECT_EQ(ss.str(), expected);
}
