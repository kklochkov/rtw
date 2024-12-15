#include "sw_renderer/barycentric.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Barycentric, default_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f B;
  EXPECT_THAT(B, ::testing::ElementsAre(0, 0, 0));
}

TEST(Barycentric, init_list_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<float, 3, 1> M{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f B{M};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector3f V{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f B{V};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_ctor)
{
  constexpr rtw::sw_renderer::Barycentric3f B1{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f B2{B1};
  EXPECT_THAT(B2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_ctor)
{
  rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::sw_renderer::Barycentric3f b2{std::move(b1)};
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_assign)
{
  constexpr rtw::sw_renderer::Barycentric3f B1{1.0F, 2.0F, 3.0F};
  rtw::sw_renderer::Barycentric3f b2;
  b2 = B1;
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_assign)
{
  rtw::sw_renderer::Barycentric3f b1{1.0F, 2.0F, 3.0F};
  rtw::sw_renderer::Barycentric3f b2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
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
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(B[0], 1);
  EXPECT_EQ(B[1], 2);
  EXPECT_EQ(B[2], 3);
}

TEST(Barycentric, operator_brackets_out_of_range)
{
  rtw::sw_renderer::Barycentric3f b{1.0F, 2.0F, 3.0F};
  b[0] = 4;
  EXPECT_DEATH(b[3], "");
}

TEST(Barycentric, operator_brackets_const_out_of_range)
{
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  EXPECT_DEATH(B[3], "");
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
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(B.alpha(), 1);
  EXPECT_EQ(B.beta(), 2);
  EXPECT_EQ(B.gamma(), 3);
}

TEST(Barycentric, operator_cast_to_vector)
{
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  constexpr auto V = static_cast<rtw::math::Vector3f>(B);
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, as_matrix)
{
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  constexpr auto M = B.as_matrix();
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, operator_equal)
{
  constexpr rtw::sw_renderer::Barycentric3f B1{1.0F, 2.0F, 3.0F};
  constexpr rtw::sw_renderer::Barycentric3f B2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(B1 == B2);
  EXPECT_FALSE(B1 != B2);
}

TEST(Barycentric, operator_stream)
{
  constexpr rtw::sw_renderer::Barycentric3f B{1.0F, 2.0F, 3.0F};
  std::stringstream ss;
  ss << B;
  constexpr auto EXPECTED = R"(Barycentric3[1.0000 2.0000 3.0000])";
  EXPECT_EQ(ss.str(), EXPECTED);
}
