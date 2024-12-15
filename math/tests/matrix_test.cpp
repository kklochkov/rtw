#include "math/matrix.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Matrix, default_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 2> M;
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0));
}

TEST(Matrix, zero_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{rtw::math::ZERO};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0));
}

TEST(Matrix, identity_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{rtw::math::IDENTITY};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 1));
}

TEST(Matrix, init_list_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, copy_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{M1};
  EXPECT_EQ(M2.rows(), 2);
  EXPECT_EQ(M2.cols(), 2);

  EXPECT_THAT(M2, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, move_ctor)
{
  rtw::math::Matrix<int, 2, 2> m1{1, 2, 3, 4};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::math::Matrix<int, 2, 2> m2{std::move(m1)};
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, copy_assign)
{
  const rtw::math::Matrix<int, 2, 2> m1{1, 2, 3, 4};
  rtw::math::Matrix<int, 2, 2> m2;
  m2 = m1;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, move_assign)
{
  rtw::math::Matrix<int, 2, 2> m1{1, 2, 3, 4};
  rtw::math::Matrix<int, 2, 2> m2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  m2 = std::move(m1);
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, operator_parenthesis)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_EQ(m(0, 0), 5);
  EXPECT_EQ(m(0, 1), 6);
  EXPECT_EQ(m(1, 0), 7);
  EXPECT_EQ(m(1, 1), 8);
}

TEST(Matrix, operator_parenthesis_const)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  EXPECT_EQ(M(0, 0), 1);
  EXPECT_EQ(M(0, 1), 2);
  EXPECT_EQ(M(1, 0), 3);
  EXPECT_EQ(M(1, 1), 4);
}

TEST(Matrix, operator_parenthesis_out_of_range)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_DEATH(m(2, 0), "");
  EXPECT_DEATH(m(0, 2), "");
}

TEST(Matrix, operator_parenthesis_const_out_of_range)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  EXPECT_DEATH(M(2, 0), "");
  EXPECT_DEATH(M(0, 2), "");
}

TEST(Matrix, operator_subscript)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m[0] = 5;
  m[1] = 6;
  m[2] = 7;
  m[3] = 8;
  EXPECT_EQ(m[0], 5);
  EXPECT_EQ(m[1], 6);
  EXPECT_EQ(m[2], 7);
  EXPECT_EQ(m[3], 8);
}

TEST(Matrix, operator_subscript_const)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  EXPECT_EQ(M[0], 1);
  EXPECT_EQ(M[1], 2);
  EXPECT_EQ(M[2], 3);
  EXPECT_EQ(M[3], 4);
}

TEST(Matrix, operator_subscript_out_of_range)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m[0] = 5;
  EXPECT_DEATH(m[4], "");
}

TEST(Matrix, operator_subscript_const_out_of_range)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  EXPECT_DEATH(M[4], "");
}

TEST(Matrix, operator_plus_equal)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m += rtw::math::Matrix<int, 2, 2>{1, 1, 1, 1};
  EXPECT_THAT(m, ::testing::ElementsAre(2, 3, 4, 5));
}

TEST(Matrix, operator_minus_equal)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m -= rtw::math::Matrix<int, 2, 2>{1, 1, 1, 1};
  EXPECT_THAT(m, ::testing::ElementsAre(0, 1, 2, 3));
}

TEST(Matrix, operator_multiply_equal)
{
  rtw::math::Matrix<int, 2, 2> m{1, 2, 3, 4};
  m *= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(2, 4, 6, 8));
}

TEST(Matrix, operator_divide_equal)
{
  rtw::math::Matrix<int, 2, 2> m{2, 4, 6, 8};
  m /= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, operator_plus)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{5, 6, 7, 8};
  const rtw::math::Matrix<int, 2, 2> m3 = M1 + M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(6, 8, 10, 12));
}

TEST(Matrix, operator_minus)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{5, 6, 7, 8};
  const rtw::math::Matrix<int, 2, 2> m3 = M1 - M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(-4, -4, -4, -4));
}

TEST(Matrix, operator_multiply)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{5, 6, 7, 8};
  const rtw::math::Matrix<int, 2, 2> m3 = M1 * M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(19, 22, 43, 50));
}

TEST(Matrix, operator_multiply_scalar)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  const rtw::math::Matrix<int, 2, 2> m2 = M1 * 2;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(2, 4, 6, 8));
}

TEST(Matrix, operator_divide_scalar)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{2, 4, 6, 8};
  const rtw::math::Matrix<int, 2, 2> m2 = M1 / 2;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(Matrix, operator_equal)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{1, 2, 3, 4};
  EXPECT_TRUE(M1 == M2);
}

TEST(Matrix, operator_not_equal)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  constexpr rtw::math::Matrix<int, 2, 2> M2{5, 6, 7, 8};
  EXPECT_TRUE(M1 != M2);
}

TEST(Matrix, operator_stream)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  std::stringstream ss;
  ss << M;
  constexpr auto EXPECTED = R"(Matrix2x2[
         1          2
         3          4
])";
  EXPECT_EQ(ss.str(), EXPECTED);
}

TEST(Matrix, identity)
{
  constexpr auto M = rtw::math::Matrix<int, 2, 2>::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 1));
}

TEST(Matrix, identity_3x3)
{
  constexpr auto M = rtw::math::Matrix<int, 3, 3>::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0, 0, 0, 1));
}

TEST(Matrix, identity_4x4)
{
  constexpr auto M = rtw::math::Matrix<int, 4, 4>::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
}

TEST(Matrix, transpose)
{
  constexpr rtw::math::Matrix<int, 2, 3> M1{1, 2, 3, 4, 5, 6};
  const rtw::math::Matrix<int, 3, 2> m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1, 4, 2, 5, 3, 6));
}

TEST(Matrix, transpose_3x3)
{
  constexpr rtw::math::Matrix<int, 3, 3> M1{1, 2, 3, 4, 5, 6, 7, 8, 9};
  const rtw::math::Matrix<int, 3, 3> m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1, 4, 7, 2, 5, 8, 3, 6, 9));
}

TEST(Matrix, transpose_4x4)
{
  constexpr rtw::math::Matrix<int, 4, 4> M1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  const rtw::math::Matrix<int, 4, 4> m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16));
}

TEST(Matrix, determinant_2x2)
{
  constexpr rtw::math::Matrix<int, 2, 2> M{1, 2, 3, 4};
  constexpr auto D = rtw::math::determinant(M);
  EXPECT_EQ(D, -2);
}

TEST(Matrix, determinant_3x3)
{
  constexpr rtw::math::Matrix<int, 3, 3> M{1, 2, 3, 4, 5, 6, 7, 8, 9};
  constexpr auto D = rtw::math::determinant(M);
  EXPECT_EQ(D, 0);
}

TEST(Matrix, unary_minus)
{
  constexpr rtw::math::Matrix<int, 2, 2> M1{1, 2, 3, 4};
  const rtw::math::Matrix<int, 2, 2> m2 = -M1;
  EXPECT_THAT(m2, ::testing::ElementsAre(-1, -2, -3, -4));
}

TEST(Matrix, inverse_2d)
{
  constexpr rtw::math::Matrix<float, 2, 2> M1{1.0F, 2.0F, 3.0F, 4.0F};
  const rtw::math::Matrix<float, 2, 2> m2 = rtw::math::inverse(M1);
  constexpr auto IDENTITY = rtw::math::Matrix<float, 2, 2>::identity();
  EXPECT_EQ(M1 * m2, IDENTITY);
}

TEST(Matrix, inverse_3d)
{
  constexpr rtw::math::Matrix<float, 3, 3> M1{1.0F, 0.0F, 0.0F, 0.0F, 2.0F, 0.0F, 0.0F, 0.0F, 4.0F};
  const rtw::math::Matrix<float, 3, 3> m2 = rtw::math::inverse(M1);
  constexpr auto IDENTITY = rtw::math::Matrix<float, 3, 3>::identity();
  EXPECT_EQ(M1 * m2, IDENTITY);
}
