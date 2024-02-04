#include "sw_renderer/barycentric.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Barycentric, default_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f b;
  EXPECT_THAT(b, ::testing::ElementsAre(0, 0, 0));
}

TEST(Barycentric, init_list_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  EXPECT_THAT(b, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<float, 3, 1> m{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f b{m};
  EXPECT_THAT(b, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector3f v{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f b{v};
  EXPECT_THAT(b, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f b2{b1};
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_ctor)
{
  rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  const rtw::sw_renderer::Barycentric3f b2{std::move(b1)};
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_assign)
{
  constexpr rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  rtw::sw_renderer::Barycentric3f b2;
  b2 = b1;
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_assign)
{
  rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  rtw::sw_renderer::Barycentric3f b2;
  b2 = std::move(b1);
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, operator_brackets)
{
  rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  b[0] = 4.0F;
  b[1] = 5.0F;
  b[2] = 6.0F;
  EXPECT_EQ(b[0], 4);
  EXPECT_EQ(b[1], 5);
  EXPECT_EQ(b[2], 6);
}

TEST(Barycentric, operator_brackets_const)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(b[0], 1);
  EXPECT_EQ(b[1], 2);
  EXPECT_EQ(b[2], 3);
}

TEST(Barycentric, operator_brackets_out_of_range)
{
  rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  b[0] = 4;
  EXPECT_DEATH(b[3], "");
}

TEST(Barycentric, operator_brackets_const_out_of_range)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  EXPECT_DEATH(b[3], "");
}

TEST(Barycentric, accessors)
{
  rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(b.alpha(), 1);
  EXPECT_EQ(b.beta(), 2);
  EXPECT_EQ(b.gamma(), 3);

  b.alpha() = 4.0F;
  b.beta() = 5.0F;
  b.gamma() = 6.0F;
  EXPECT_EQ(b.alpha(), 4);
  EXPECT_EQ(b.beta(), 5);
  EXPECT_EQ(b.gamma(), 6);
}

TEST(Barycentric, accessors_const)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(b.alpha(), 1);
  EXPECT_EQ(b.beta(), 2);
  EXPECT_EQ(b.gamma(), 3);
}

TEST(Barycentric, operator_cast_to_vector)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  constexpr auto v = static_cast<rtw::math::Vector3f>(b);
  EXPECT_THAT(v, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, as_matrix)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  constexpr auto m = b.as_matrix();
  EXPECT_EQ(m.rows(), 3);
  EXPECT_EQ(m.cols(), 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, operator_equal)
{
  constexpr rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f b2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(b1 == b2);
  EXPECT_FALSE(b1 != b2);
}

TEST(Barycentric, operator_stream)
{
  constexpr rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  std::stringstream ss;
  ss << b;
  constexpr auto expected = R"(Barycentric3[1.0000 2.0000 3.0000])";
  EXPECT_EQ(ss.str(), expected);
}
