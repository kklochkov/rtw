#include "math/matrix.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

constexpr auto EPSILON = 1.0e-3F;

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
  constexpr auto IDENTITY = rtw::math::Matrix<float, 2, 2>::identity();
  {
    const rtw::math::Matrix<float, 2, 2> m2 = rtw::math::inverse(M1);
    EXPECT_EQ(M1 * m2, IDENTITY);
  }
  {
    const rtw::math::Matrix<float, 2, 2> m2 = rtw::math::householder::qr::inverse(M1);
    const auto result = M1 * m2;
    for (std::uint32_t i = 0U; i < result.size(); ++i)
    {
      EXPECT_NEAR(result[i], IDENTITY[i], EPSILON);
    }
  }
}

TEST(Matrix, inverse_3d)
{
  constexpr rtw::math::Matrix<float, 3, 3> M1{1.0F, 0.0F, 0.0F, 0.0F, 2.0F, 0.0F, 0.0F, 0.0F, 4.0F};
  constexpr auto IDENTITY = rtw::math::Matrix<float, 3, 3>::identity();
  {
    const rtw::math::Matrix<float, 3, 3> m2 = rtw::math::inverse(M1);
    EXPECT_EQ(M1 * m2, IDENTITY);
  }
  {
    const rtw::math::Matrix<float, 3, 3> m2 = rtw::math::householder::qr::inverse(M1);
    const auto result = M1 * m2;
    for (std::uint32_t i = 0U; i < result.size(); ++i)
    {
      EXPECT_NEAR(result[i], IDENTITY[i], EPSILON);
    }
  }
}

TEST(Matrix, minor)
{
  constexpr rtw::math::Matrix<int, 3, 3> M{1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto m = M.minor(0, 0);
  EXPECT_THAT(m, ::testing::ElementsAre(5, 6, 8, 9));

  m = M.minor(1, 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 3, 7, 9));

  m = M.minor(2, 2);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2, 4, 5));
}

TEST(Matrix, householder_qr_decomposition)
{
  constexpr rtw::math::Matrix<float, 3, 3> A{12.0F, -51.0F, 4.0F, 6.0F, 167.0F, -68.0F, -4.0F, 24.0F, -41.0F};
  constexpr rtw::math::Matrix<float, 3, 3> EXPECTED_Q{-0.8571F, -0.4286F, 0.2857F,  0.3943F, -0.9029F,
                                                      -0.1714F, 0.3314F,  -0.0343F, 0.9429F};
  constexpr rtw::math::Matrix<float, 3, 3> EXPECTED_R{-14.0000F, -21.0000F, 14.0000F, 0.0000F,  -175.0000F,
                                                      70.0000F,  0.0000F,   -0.0000F, -35.0000F};
  const auto [q, r] = rtw::math::householder::qr::decompose(A);
  for (std::uint32_t i = 0U; i < EXPECTED_Q.size(); ++i)
  {
    EXPECT_NEAR(EXPECTED_Q[i], q[i], EPSILON);
  }
  for (std::uint32_t i = 0U; i < EXPECTED_R.size(); ++i)
  {
    EXPECT_NEAR(EXPECTED_R[i], r[i], EPSILON);
  }

  const auto qr = rtw::math::transpose(q) * r;
  for (std::uint32_t i = 0U; i < A.size(); ++i)
  {
    EXPECT_NEAR(A[i], qr[i], EPSILON);
  }
}

TEST(Matrix, householder_qr_inverse)
{
  constexpr auto IDENTITY = rtw::math::Matrix<float, 5, 5>::identity();

  constexpr rtw::math::Matrix<float, 5, 5> A{12.0F, -51.0F, 4.0F,  7.0F,   -2.0F, 6.0F,  167.0F, -68.0F, -3.0F,
                                             5.0F,  -4.0F,  24.0F, -41.0F, 2.0F,  9.0F,  5.0F,   -6.0F,  7.0F,
                                             14.0F, -10.0F, -2.0F, 8.0F,   -3.0F, 11.0F, 6.0F};

  const auto identity = rtw::math::inverse(A) * A;
  for (std::uint32_t i = 0U; i < identity.size(); ++i)
  {
    EXPECT_NEAR(identity[i], IDENTITY[i], EPSILON);
  }
}

TEST(Matrix, householder_qr_solve)
{
  constexpr rtw::math::Matrix<float, 3, 3> A{12.0F, -51.0F, 4.0F, 6.0F, 167.0F, -68.0F, -4.0F, 24.0F, -41.0F};
  constexpr rtw::math::Matrix<float, 3, 1> B{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Matrix<float, 3, 1> EXPECTED_X{0.0094F, -0.0243F, -0.0883F};

  const auto x = rtw::math::householder::qr::solve(A, B);
  for (std::uint32_t i = 0U; i < x.size(); ++i)
  {
    EXPECT_NEAR(x[i], EXPECTED_X[i], EPSILON);
  }

  const auto b = A * x;
  for (std::uint32_t i = 0U; i < b.size(); ++i)
  {
    EXPECT_NEAR(b[i], B[i], EPSILON);
  }
}
