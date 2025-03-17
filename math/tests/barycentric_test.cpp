#include "math/barycentric.h"
#include "math/format.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>

TEST(Barycentric, default_ctor)
{
  constexpr rtw::math::Barycentric3F B;
  EXPECT_THAT(B, ::testing::ElementsAre(0, 0, 0));
}

TEST(Barycentric, init_list_ctor)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_matrix)
{
  constexpr rtw::math::Matrix<float, 3, 1> M{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Barycentric3F B{M};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, conversion_ctor_from_vector)
{
  constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Barycentric3F B{V};
  EXPECT_THAT(B, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_ctor)
{
  constexpr rtw::math::Barycentric3F B1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Barycentric3F B2{B1};
  EXPECT_THAT(B2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_ctor)
{
  rtw::math::Barycentric3F b1{1.0F, 2.0F, 3.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::math::Barycentric3F b2{std::move(b1)};
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, copy_assign)
{
  constexpr rtw::math::Barycentric3F B1{1.0F, 2.0F, 3.0F};
  rtw::math::Barycentric3F b2;
  b2 = B1;
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, move_assign)
{
  rtw::math::Barycentric3F b1{1.0F, 2.0F, 3.0F};
  rtw::math::Barycentric3F b2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  b2 = std::move(b1);
  EXPECT_THAT(b2, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, operator_brackets)
{
  rtw::math::Barycentric3F b{1.0F, 2.0F, 3.0F};
  b[0] = 4.0F;
  b[1] = 5.0F;
  b[2] = 6.0F;
  EXPECT_EQ(b[0], 4);
  EXPECT_EQ(b[1], 5);
  EXPECT_EQ(b[2], 6);
}

TEST(Barycentric, operator_brackets_const)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(B[0], 1);
  EXPECT_EQ(B[1], 2);
  EXPECT_EQ(B[2], 3);
}

TEST(Barycentric, operator_brackets_out_of_range)
{
  rtw::math::Barycentric3F b{1.0F, 2.0F, 3.0F};
  b[0] = 4;
  EXPECT_DEATH(b[3], "");
}

TEST(Barycentric, operator_brackets_const_out_of_range)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  EXPECT_DEATH(B[3], "");
}

TEST(Barycentric, accessors)
{
  rtw::math::Barycentric3F b{1.0F, 2.0F, 3.0F};
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
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  EXPECT_EQ(B.alpha(), 1);
  EXPECT_EQ(B.beta(), 2);
  EXPECT_EQ(B.gamma(), 3);
}

TEST(Barycentric, operator_cast_to_vector)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  constexpr auto V = static_cast<rtw::math::Vector3F>(B);
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, as_matrix)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  constexpr auto M = B.as_matrix();
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2, 3));
}

TEST(Barycentric, operator_equal)
{
  constexpr rtw::math::Barycentric3F B1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Barycentric3F B2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(B1 == B2);
  EXPECT_FALSE(B1 != B2);
}

TEST(Barycentric, operator_stream)
{
  constexpr rtw::math::Barycentric3F B{1.0F, 2.0F, 3.0F};
  std::stringstream ss;
  ss << B;
  constexpr auto EXPECTED = R"(Barycentric3[1.0000 2.0000 3.0000])";
  EXPECT_EQ(ss.str(), EXPECTED);
}

TEST(Barycentric, make_baricentric)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  const std::array<std::pair<rtw::math::Point2F, rtw::math::Barycentric3F>, 9> data = {
      std::pair{rtw::math::Point2F{0.0F, 0.0F}, rtw::math::Barycentric3F{1.0F, 0.0F, 0.0F}},
      std::pair{rtw::math::Point2F{0.5F, 0.0F}, rtw::math::Barycentric3F{0.5F, 0.5F, 0.0F}},
      std::pair{rtw::math::Point2F{0.0F, 0.5F}, rtw::math::Barycentric3F{0.5F, 0.0F, 0.5F}},
      std::pair{rtw::math::Point2F{0.5F, 0.5F}, rtw::math::Barycentric3F{0.0F, 0.5F, 0.5F}},
      std::pair{rtw::math::Point2F{0.4F, 0.4F}, rtw::math::Barycentric3F{0.2F, 0.4F, 0.4F}},
      std::pair{rtw::math::Point2F{-0.5F, 0.5F}, rtw::math::Barycentric3F{1.0F, -0.5F, 0.5F}},
      std::pair{rtw::math::Point2F{-0.5F, -0.5F}, rtw::math::Barycentric3F{2.0F, -0.5F, -0.5F}},
      std::pair{rtw::math::Point2F{0.5F, -0.5F}, rtw::math::Barycentric3F{1.0F, 0.5F, -0.5F}},
      std::pair{rtw::math::Point2F{1.0F, 1.0F}, rtw::math::Barycentric3F{-1.0F, 1.0F, 1.0F}},
  };

  for (const auto& [p, b] : data)
  {
    const auto result = rtw::math::make_barycentric(v0, v1, v2, p);
    ASSERT_FLOAT_EQ(result.alpha(), b.alpha());
    ASSERT_FLOAT_EQ(result.beta(), b.beta());
    ASSERT_FLOAT_EQ(result.gamma(), b.gamma());
  }
}

TEST(Barycentric, contains)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  const std::array<std::pair<rtw::math::Point2F, bool>, 9> data = {
      std::pair{rtw::math::Point2F{0.0F, 0.0F}, true},    std::pair{rtw::math::Point2F{0.5F, 0.0F}, true},
      std::pair{rtw::math::Point2F{0.0F, 0.5F}, true},    std::pair{rtw::math::Point2F{0.5F, 0.5F}, true},
      std::pair{rtw::math::Point2F{0.4F, 0.4F}, true},    std::pair{rtw::math::Point2F{-0.5F, 0.5F}, false},
      std::pair{rtw::math::Point2F{-0.5F, -0.5F}, false}, std::pair{rtw::math::Point2F{0.5F, -0.5F}, false},
      std::pair{rtw::math::Point2F{1.0F, 1.0F}, false},
  };

  for (const auto& [p, b] : data)
  {
    const auto result = rtw::math::contains(v0, v1, v2, p);
    ASSERT_EQ(result, b);
  }
}

TEST(Barycentric, contains2)
{
  const rtw::math::Point2I a{80, 100};
  const rtw::math::Point2I b{200, 100};
  const rtw::math::Point2I c{150, 200};
  const rtw::math::Point2I p{150, 150};

  const auto r = rtw::math::make_barycentric(a.cast<float>(), b.cast<float>(), c.cast<float>(), p.cast<float>());
  ASSERT_FLOAT_EQ(r.alpha(), 5.0F / 24.0F);
  ASSERT_FLOAT_EQ(r.beta(), 7.0F / 24.0F);
  ASSERT_FLOAT_EQ(r.gamma(), 1.0F / 2.0F);
}

TEST(Barycentric, contains3)
{
  const rtw::math::Point2I a{50, 50};
  const rtw::math::Point2I b{100, 150};
  const rtw::math::Point2I c{250, 250};
  const rtw::math::Point2I p{106, 154};

  const auto r = rtw::math::make_barycentric(a.cast<float>(), b.cast<float>(), c.cast<float>(), p.cast<float>());
  ASSERT_FLOAT_EQ(r.alpha(), 0.0F);
  ASSERT_NEAR(r.beta(), 24.F / 25.0F, 1e-6F);
  ASSERT_NEAR(r.gamma(), 1.0F / 25.0F, 1e-6F);
}
