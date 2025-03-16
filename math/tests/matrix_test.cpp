#include "math/matrix.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// FixedPoint16 has lower precision and requires a larger epsilon
template <typename T>
constexpr auto EPSILON = std::is_same_v<typename T::value_type, rtw::fixed_point::FixedPoint16> ? 7.0e-3F : 1.0e-4F;

template <typename T>
class Matrix2x2Test : public ::testing::Test
{};

using Matrix2x2Types =
    ::testing::Types<rtw::math::Matrix2x2F, rtw::math::Matrix2x2D, rtw::math::Matrix2x2Q16, rtw::math::Matrix2x2Q32>;
TYPED_TEST_SUITE(Matrix2x2Test, Matrix2x2Types, );

TYPED_TEST(Matrix2x2Test, default_ctor)
{
  constexpr TypeParam M;
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0));
}

TYPED_TEST(Matrix2x2Test, zero_ctor)
{
  constexpr TypeParam M{rtw::math::ZERO};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0));
}

TYPED_TEST(Matrix2x2Test, identity_ctor)
{
  constexpr TypeParam M{rtw::math::IDENTITY};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 1));
}

TYPED_TEST(Matrix2x2Test, initialize_with_value_ctor)
{
  constexpr TypeParam M{rtw::math::INITIALIZE_WITH_VALUE, 42.0F};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(42.0F, 42.0F, 42.0F, 42.0F));
}

TYPED_TEST(Matrix2x2Test, init_list_ctor)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 2);

  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, copy_ctor)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{M1};
  EXPECT_EQ(M2.rows(), 2);
  EXPECT_EQ(M2.cols(), 2);

  EXPECT_THAT(M2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, move_ctor)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const TypeParam m2{std::move(m1)};
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, copy_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  m2 = m1;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, move_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  m2 = std::move(m1);
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, operator_parenthesis)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_EQ(m(0, 0), 5);
  EXPECT_EQ(m(0, 1), 6);
  EXPECT_EQ(m(1, 0), 7);
  EXPECT_EQ(m(1, 1), 8);
}

TYPED_TEST(Matrix2x2Test, operator_parenthesis_const)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_EQ(M(0, 0), 1);
  EXPECT_EQ(M(0, 1), 2);
  EXPECT_EQ(M(1, 0), 3);
  EXPECT_EQ(M(1, 1), 4);
}

TYPED_TEST(Matrix2x2Test, operator_parenthesis_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_DEATH(m(2, 0), "");
  EXPECT_DEATH(m(0, 2), "");
}

TYPED_TEST(Matrix2x2Test, operator_parenthesis_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_DEATH(M(2, 0), "");
  EXPECT_DEATH(M(0, 2), "");
}

TYPED_TEST(Matrix2x2Test, operator_subscript)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m[0] = 5;
  m[1] = 6;
  m[2] = 7;
  m[3] = 8;
  EXPECT_EQ(m[0], 5);
  EXPECT_EQ(m[1], 6);
  EXPECT_EQ(m[2], 7);
  EXPECT_EQ(m[3], 8);
}

TYPED_TEST(Matrix2x2Test, operator_subscript_const)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_EQ(M[0], 1);
  EXPECT_EQ(M[1], 2);
  EXPECT_EQ(M[2], 3);
  EXPECT_EQ(M[3], 4);
}

TYPED_TEST(Matrix2x2Test, operator_subscript_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m[0] = 5;
  EXPECT_DEATH(m[4], "");
}

TYPED_TEST(Matrix2x2Test, operator_subscript_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_DEATH(M[4], "");
}

TYPED_TEST(Matrix2x2Test, operator_plus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m += TypeParam{1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 3.0F, 4.0F, 5.0F));
}

TYPED_TEST(Matrix2x2Test, operator_minus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m -= TypeParam{1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(0.0F, 1.0F, 2.0F, 3.0F));
}

TYPED_TEST(Matrix2x2Test, operator_multiply_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F};
  m *= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F));
}

TYPED_TEST(Matrix2x2Test, operator_divide_equal)
{
  TypeParam m{2.0F, 4.0F, 6.0F, 8.0F};
  m /= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
}

TYPED_TEST(Matrix2x2Test, operator_plus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F};
  const auto m3 = M1 + M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(6.0F, 8.0F, 10.0F, 12.0F));
}

TYPED_TEST(Matrix2x2Test, operator_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F};
  const auto m3 = M1 - M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(-4.0F, -4.0F, -4.0F, -4.0F));
}

TYPED_TEST(Matrix2x2Test, operator_multiply)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F};
  const auto m3 = M1 * M2;
  EXPECT_EQ(m3.rows(), 2);
  EXPECT_EQ(m3.cols(), 2);

  EXPECT_THAT(m3, ::testing::ElementsAre(19.0F, 22.0F, 43.0F, 50.0F));
}

TYPED_TEST(Matrix2x2Test, operator_multiply_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  const auto m2 = M1 * 2;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F));
}

TYPED_TEST(Matrix2x2Test, operator_divide_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  const auto m2 = M1 / 2;
  EXPECT_EQ(m2.rows(), 2);
  EXPECT_EQ(m2.cols(), 2);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F / 2.0F, 2.0F / 2.0F, 3.0F / 2.0F, 4.0F / 2.0F));
}

TYPED_TEST(Matrix2x2Test, operator_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{1.0F, 2.0F, 3.0F, 4.0F};
  EXPECT_TRUE(M1 == M2);
}

TYPED_TEST(Matrix2x2Test, operator_not_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F};
  EXPECT_TRUE(M1 != M2);
}

TYPED_TEST(Matrix2x2Test, identity)
{
  constexpr auto M = TypeParam::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 1));
}

TYPED_TEST(Matrix2x2Test, transpose)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  const auto m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1, 3, 2, 4));
}

TYPED_TEST(Matrix2x2Test, determinant)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr auto D = rtw::math::determinant(M);
  EXPECT_EQ(D, -2);
}

TYPED_TEST(Matrix2x2Test, unary_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  const auto m2 = -M1;
  EXPECT_THAT(m2, ::testing::ElementsAre(-1, -2, -3, -4));
}

TYPED_TEST(Matrix2x2Test, inverse)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F};
  constexpr auto IDENTITY = TypeParam::identity();
  {
    const auto m2 = rtw::math::inverse(M1);
    EXPECT_EQ(M1 * m2, IDENTITY);
  }
  {
    const auto m2 = rtw::math::matrix_decomposition::qr::householder::inverse(M1);
    const auto result = M1 * m2;
    for (std::uint32_t i = 0U; i < result.size(); ++i)
    {
      EXPECT_NEAR(static_cast<double>(result[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
    }
  }
}
//-----------------------------------------------------------------------------------------
template <typename T>
class Matrix3x3Test : public ::testing::Test
{
public:
  void evaluate_decomposition(const T& expected_a, const T& expected_q, const T& expected_r, const T& q, const T& r,
                              const float epsilon = EPSILON<T>) const
  {
    for (std::uint32_t i = 0U; i < expected_q.size(); ++i)
    {
      EXPECT_NEAR(static_cast<double>(expected_q[i]), static_cast<double>(q[i]), epsilon);
    }

    for (std::uint32_t row = 0U; row < expected_q.rows(); ++row)
    {
      const auto norm = rtw::math::norm(q.row(row));
      const auto expected_norm = rtw::math::norm(expected_q.row(row));

      EXPECT_NEAR(static_cast<double>(expected_norm), static_cast<double>(norm), epsilon);
    }

    for (std::uint32_t i = 0U; i < expected_r.size(); ++i)
    {
      EXPECT_NEAR(static_cast<double>(expected_r[i]), static_cast<double>(r[i]), epsilon);
    }

    const auto qr = rtw::math::transpose(q) * r;
    for (std::uint32_t i = 0U; i < expected_a.size(); ++i)
    {
      EXPECT_NEAR(static_cast<double>(expected_a[i]), static_cast<double>(qr[i]), epsilon);
    }
  }

  using Vector = rtw::math::Matrix<typename T::value_type, 3, 1>;

  void evaluate_solve(const T& expected_a, const Vector& expected_b, const Vector& expected_x, const Vector& x,
                      const float epsilon = EPSILON<T>)
  {
    {
      for (std::uint32_t i = 0U; i < x.size(); ++i)
      {
        EXPECT_NEAR(static_cast<double>(x[i]), static_cast<double>(expected_x[i]), epsilon);
      }

      const auto b = expected_a * x;
      for (std::uint32_t i = 0U; i < b.size(); ++i)
      {
        EXPECT_NEAR(static_cast<double>(b[i]), static_cast<double>(expected_b[i]), epsilon);
      }
    }
  }
};

using Matrix3x3Types =
    ::testing::Types<rtw::math::Matrix3x3F, rtw::math::Matrix3x3D, rtw::math::Matrix3x3Q16, rtw::math::Matrix3x3Q32>;
TYPED_TEST_SUITE(Matrix3x3Test, Matrix3x3Types, );

TYPED_TEST(Matrix3x3Test, default_ctor)
{
  constexpr TypeParam M;
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 3);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0));
}

TYPED_TEST(Matrix3x3Test, zero_ctor)
{
  constexpr TypeParam M{rtw::math::ZERO};
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 3);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0));
}

TYPED_TEST(Matrix3x3Test, identity_ctor)
{
  constexpr TypeParam M{rtw::math::IDENTITY};
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 3);

  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0, 0, 0, 1));
}

TYPED_TEST(Matrix3x3Test, initialize_with_value_ctor)
{
  constexpr TypeParam M{rtw::math::INITIALIZE_WITH_VALUE, 42.0F};
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 3);

  EXPECT_THAT(M, ::testing::ElementsAre(42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F));
}

TYPED_TEST(Matrix3x3Test, init_list_ctor)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_EQ(M.rows(), 3);
  EXPECT_EQ(M.cols(), 3);

  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, copy_ctor)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{M1};
  EXPECT_EQ(M2.rows(), 3);
  EXPECT_EQ(M2.cols(), 3);

  EXPECT_THAT(M2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, move_ctor)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const TypeParam m2{std::move(m1)};
  EXPECT_EQ(m2.rows(), 3);
  EXPECT_EQ(m2.cols(), 3);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, copy_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  m2 = m1;
  EXPECT_EQ(m2.rows(), 3);
  EXPECT_EQ(m2.cols(), 3);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, move_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  m2 = std::move(m1);
  EXPECT_EQ(m2.rows(), 3);
  EXPECT_EQ(m2.cols(), 3);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, operator_parenthesis)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_EQ(m(0, 0), 5);
  EXPECT_EQ(m(0, 1), 6);
  EXPECT_EQ(m(1, 0), 7);
  EXPECT_EQ(m(1, 1), 8);
}

TYPED_TEST(Matrix3x3Test, operator_parenthesis_const)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_EQ(M(0, 0), 1);
  EXPECT_EQ(M(0, 1), 2);
  EXPECT_EQ(M(1, 0), 4);
  EXPECT_EQ(M(1, 1), 5);
}

TYPED_TEST(Matrix3x3Test, operator_parenthesis_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_DEATH(m(3, 0), "");
  EXPECT_DEATH(m(0, 3), "");
}

TYPED_TEST(Matrix3x3Test, operator_parenthesis_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_DEATH(M(3, 0), "");
  EXPECT_DEATH(M(0, 3), "");
}

TYPED_TEST(Matrix3x3Test, operator_subscript)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m[0] = 5;
  m[1] = 6;
  m[2] = 7;
  m[3] = 8;
  EXPECT_EQ(m[0], 5);
  EXPECT_EQ(m[1], 6);
  EXPECT_EQ(m[2], 7);
  EXPECT_EQ(m[3], 8);
}

TYPED_TEST(Matrix3x3Test, operator_subscript_const)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_EQ(M[0], 1);
  EXPECT_EQ(M[1], 2);
  EXPECT_EQ(M[2], 3);
  EXPECT_EQ(M[3], 4);
}

TYPED_TEST(Matrix3x3Test, operator_subscript_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m[0] = 5;
  EXPECT_DEATH(m[10], "");
}

TYPED_TEST(Matrix3x3Test, operator_subscript_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_DEATH(M[10], "");
}

TYPED_TEST(Matrix3x3Test, operator_plus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m += TypeParam{1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F));
}

TYPED_TEST(Matrix3x3Test, operator_minus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m -= TypeParam{1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F));
}

TYPED_TEST(Matrix3x3Test, operator_multiply_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  m *= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F));
}

TYPED_TEST(Matrix3x3Test, operator_divide_equal)
{
  TypeParam m{2.0F, 4.0F, 6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F};
  m /= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F));
}

TYPED_TEST(Matrix3x3Test, operator_plus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F};
  const auto m3 = M1 + M2;
  EXPECT_EQ(m3.rows(), 3);
  EXPECT_EQ(m3.cols(), 3);

  EXPECT_THAT(m3, ::testing::ElementsAre(6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F, 20.0F, 22.0F));
}

TYPED_TEST(Matrix3x3Test, operator_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F};
  const auto m3 = M1 - M2;
  EXPECT_EQ(m3.rows(), 3);
  EXPECT_EQ(m3.cols(), 3);

  EXPECT_THAT(m3, ::testing::ElementsAre(-4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F));
}

TYPED_TEST(Matrix3x3Test, operator_multiply)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F};
  const auto m3 = M1 * M2;
  EXPECT_EQ(m3.rows(), 3);
  EXPECT_EQ(m3.cols(), 3);

  EXPECT_THAT(m3, ::testing::ElementsAre(54.0F, 60.0F, 66.0F, 126.0F, 141.0F, 156.0F, 198.0F, 222.0F, 246.0F));
}

TYPED_TEST(Matrix3x3Test, operator_multiply_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  const auto m2 = M1 * 2;
  EXPECT_EQ(m2.rows(), 3);
  EXPECT_EQ(m2.cols(), 3);

  EXPECT_THAT(m2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F));
}

TYPED_TEST(Matrix3x3Test, operator_divide_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  const auto m2 = M1 / 2;
  EXPECT_EQ(m2.rows(), 3);
  EXPECT_EQ(m2.cols(), 3);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F / 2.0F, 2.0F / 2.0F, 3.0F / 2.0F, 4.0F / 2.0F, 5.0F / 2.0F, 6.0F / 2.0F,
                                         7.0F / 2.0F, 8.0F / 2.0F, 9.0F / 2.0F));
}

TYPED_TEST(Matrix3x3Test, operator_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  EXPECT_TRUE(M1 == M2);
}

TYPED_TEST(Matrix3x3Test, operator_not_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr TypeParam M2{5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F};
  EXPECT_TRUE(M1 != M2);
}

TYPED_TEST(Matrix3x3Test, identity)
{
  constexpr auto M = TypeParam::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0, 0, 0, 1));
}

TYPED_TEST(Matrix3x3Test, transpose)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  const auto m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1, 4, 7, 2, 5, 8, 3, 6, 9));
}

TYPED_TEST(Matrix3x3Test, determinant)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr auto D = rtw::math::determinant(M);
  EXPECT_EQ(D, 0);
}

TYPED_TEST(Matrix3x3Test, unary_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  const auto m2 = -M1;
  EXPECT_THAT(m2, ::testing::ElementsAre(-1, -2, -3, -4, -5, -6, -7, -8, -9));
}

TYPED_TEST(Matrix3x3Test, inverse)
{
  constexpr TypeParam M1{1.0F, 0.0F, 0.0F, 0.0F, 2.0F, 0.0F, 0.0F, 0.0F, 4.0F};
  constexpr auto IDENTITY = TypeParam::identity();
  {
    const auto m2 = rtw::math::inverse(M1);
    EXPECT_EQ(M1 * m2, IDENTITY);
  }
  {
    const auto m2 = rtw::math::matrix_decomposition::qr::householder::inverse(M1);
    const auto result = M1 * m2;
    for (std::uint32_t i = 0U; i < result.size(); ++i)
    {
      EXPECT_NEAR(static_cast<double>(result[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
    }
  }
}

TYPED_TEST(Matrix3x3Test, minor)
{
  constexpr TypeParam M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  auto m = M.minor(0, 0);
  EXPECT_THAT(m, ::testing::ElementsAre(5, 6, 8, 9));

  m = M.minor(1, 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 3, 7, 9));

  m = M.minor(2, 2);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2, 4, 5));
}

TYPED_TEST(Matrix3x3Test, householder_qr_decomposition)
{
  constexpr TypeParam EXPECTED_A{12.0F, -51.0F, 4.0F, 6.0F, 167.0F, -68.0F, -4.0F, 24.0F, -41.0F};
  constexpr TypeParam EXPECTED_Q{-0.8571F, -0.4286F, 0.2857F, 0.3943F, -0.9029F, -0.1714F, -0.3314F, 0.0343F, -0.9429F};
  constexpr TypeParam EXPECTED_R{-14.0F, -21.0F, 14.0F, 0.0F, -175.0F, 70.0F, 0.0F, 0.0F, 35.0F};

  const auto [q, r] = rtw::math::matrix_decomposition::qr::householder::decompose(EXPECTED_A);
  this->evaluate_decomposition(EXPECTED_A, EXPECTED_Q, EXPECTED_R, q, r);
}

TYPED_TEST(Matrix3x3Test, householder_qr_solve)
{
  using Vector = rtw::math::Matrix<typename TypeParam::value_type, 3, 1>;
  constexpr TypeParam A{1.0F, 3.0F, -2.0F, 3.0F, 5.0F, 6.0F, 2.0F, 4.0F, 3.0F};
  constexpr Vector B{5.0F, 7.0F, 8.0F};
  constexpr Vector EXPECTED_X{-15.0F, 8.0F, 2.0F};

  {
    const auto x = rtw::math::matrix_decomposition::qr::householder::solve(A, B);
    this->evaluate_solve(A, B, EXPECTED_X, x);
  }
  {
    const auto a_inv = rtw::math::matrix_decomposition::qr::householder::inverse(A);
    const auto x = a_inv * B;
    this->evaluate_solve(A, B, EXPECTED_X, x);
  }
}

TYPED_TEST(Matrix3x3Test, givens_qr_decomposition)
{
  constexpr TypeParam EXPECTED_A{12.0F, -51.0F, 4.0F, 6.0F, 167.0F, -68.0F, -4.0F, 24.0F, -41.0F};
  constexpr TypeParam EXPECTED_Q{0.8571F, 0.4286F, -0.2857F, -0.3943F, 0.9029F, 0.1714F, 0.3314F, -0.0343F, 0.9429F};
  constexpr TypeParam EXPECTED_R{14.0F, 21.0F, -14.0F, 0.0F, 175.0F, -70.0F, 0.0F, 0.0F, -35.0F};

  const auto [q, r] = rtw::math::matrix_decomposition::qr::givens::decompose(EXPECTED_A);
  this->evaluate_decomposition(EXPECTED_A, EXPECTED_Q, EXPECTED_R, q, r);
}

TYPED_TEST(Matrix3x3Test, givens_qr_solve)
{
  using Vector = rtw::math::Matrix<typename TypeParam::value_type, 3, 1>;
  constexpr TypeParam A{1.0F, 3.0F, -2.0F, 3.0F, 5.0F, 6.0F, 2.0F, 4.0F, 3.0F};
  constexpr Vector B{5.0F, 7.0F, 8.0F};
  constexpr Vector EXPECTED_X{-15.0F, 8.0F, 2.0F};

  {
    const auto x = rtw::math::matrix_decomposition::qr::givens::solve(A, B);
    this->evaluate_solve(A, B, EXPECTED_X, x);
  }
  {
    const auto a_inv = rtw::math::matrix_decomposition::qr::givens::inverse(A);
    const auto x = a_inv * B;
    this->evaluate_solve(A, B, EXPECTED_X, x);
  }
}

TYPED_TEST(Matrix3x3Test, modified_gram_schmidt_qr_decomposition)
{
  constexpr TypeParam EXPECTED_A{12.0F, -51.0F, 4.0F, 6.0F, 167.0F, -68.0F, -4.0F, 24.0F, -41.0F};
  constexpr TypeParam EXPECTED_Q{0.8571F, 0.4286F, -0.2857F, -0.3943F, 0.9029F, 0.1714F, -0.3314F, 0.0343F, -0.9429F};
  constexpr TypeParam EXPECTED_R{14.0F, 21.0F, -14.0F, 0.0F, 175.0F, -70.0F, 0.0F, 0.0F, 35.0F};

  const auto [q, r] = rtw::math::matrix_decomposition::qr::modified_gram_schmidt::decompose(EXPECTED_A);
  this->evaluate_decomposition(EXPECTED_A, EXPECTED_Q, EXPECTED_R, q, r);
}

TYPED_TEST(Matrix3x3Test, modified_gram_schmidt_qr_solve)
{
  using Vector = rtw::math::Matrix<typename TypeParam::value_type, 3, 1>;
  constexpr TypeParam A{1.0F, 3.0F, -2.0F, 3.0F, 5.0F, 6.0F, 2.0F, 4.0F, 3.0F};
  constexpr Vector B{5.0F, 7.0F, 8.0F};
  constexpr Vector EXPECTED_X{-15.0F, 8.0F, 2.0F};

  float epsilon = EPSILON<TypeParam>;
  if constexpr (std::is_same_v<typename TypeParam::value_type, float>)
  {
    // On x86-64, the precision of float for the MGS algorithm is not enough to get the exact result.
    epsilon = 0.0035F;
  }

  {
    const auto x = rtw::math::matrix_decomposition::qr::modified_gram_schmidt::solve(A, B);
    this->evaluate_solve(A, B, EXPECTED_X, x, epsilon);
  }
  {
    const auto a_inv = rtw::math::matrix_decomposition::qr::modified_gram_schmidt::inverse(A);
    const auto x = a_inv * B;
    this->evaluate_solve(A, B, EXPECTED_X, x, epsilon);
  }
}
//-----------------------------------------------------------------------------------------
template <typename T>
class Matrix4x4Test : public ::testing::Test
{};

using Matrix4x4Types =
    ::testing::Types<rtw::math::Matrix4x4F, rtw::math::Matrix4x4D, rtw::math::Matrix4x4Q16, rtw::math::Matrix4x4Q32>;
TYPED_TEST_SUITE(Matrix4x4Test, Matrix4x4Types, );

TYPED_TEST(Matrix4x4Test, default_ctor)
{
  constexpr TypeParam M;
  EXPECT_EQ(M.rows(), 4);
  EXPECT_EQ(M.cols(), 4);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

TYPED_TEST(Matrix4x4Test, zero_ctor)
{
  constexpr TypeParam M{rtw::math::ZERO};
  EXPECT_EQ(M.rows(), 4);
  EXPECT_EQ(M.cols(), 4);

  EXPECT_THAT(M, ::testing::ElementsAre(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

TYPED_TEST(Matrix4x4Test, identity_ctor)
{
  constexpr TypeParam M{rtw::math::IDENTITY};
  EXPECT_EQ(M.rows(), 4);
  EXPECT_EQ(M.cols(), 4);

  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
}

TYPED_TEST(Matrix4x4Test, initialize_with_value_ctor)
{
  constexpr TypeParam M{rtw::math::INITIALIZE_WITH_VALUE, 42.0F};
  EXPECT_EQ(M.rows(), 4);
  EXPECT_EQ(M.cols(), 4);

  EXPECT_THAT(M, ::testing::ElementsAre(42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F, 42.0F,
                                        42.0F, 42.0F, 42.0F, 42.0F, 42.0F));
}

TYPED_TEST(Matrix4x4Test, init_list_ctor)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_EQ(M.rows(), 4);
  EXPECT_EQ(M.cols(), 4);

  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                        13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, copy_ctor)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{M1};
  EXPECT_EQ(M2.rows(), 4);
  EXPECT_EQ(M2.cols(), 4);

  EXPECT_THAT(M2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                         13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, move_ctor)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const TypeParam m2{std::move(m1)};
  EXPECT_EQ(m2.rows(), 4);
  EXPECT_EQ(m2.cols(), 4);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                         13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, copy_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  m2 = m1;
  EXPECT_EQ(m2.rows(), 4);
  EXPECT_EQ(m2.cols(), 4);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                         13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, move_assign)
{
  TypeParam m1{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  TypeParam m2{rtw::math::UNINITIALIZED};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  m2 = std::move(m1);
  EXPECT_EQ(m2.rows(), 4);
  EXPECT_EQ(m2.cols(), 4);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                         13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, operator_parenthesis)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_EQ(m(0, 0), 5);
  EXPECT_EQ(m(0, 1), 6);
  EXPECT_EQ(m(1, 0), 7);
  EXPECT_EQ(m(1, 1), 8);
}

TYPED_TEST(Matrix4x4Test, operator_parenthesis_const)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_EQ(M(0, 0), 1);
  EXPECT_EQ(M(0, 1), 2);
  EXPECT_EQ(M(1, 0), 5);
  EXPECT_EQ(M(1, 1), 6);
}

TYPED_TEST(Matrix4x4Test, operator_parenthesis_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m(0, 0) = 5;
  m(0, 1) = 6;
  m(1, 0) = 7;
  m(1, 1) = 8;
  EXPECT_DEATH(m(4, 0), "");
  EXPECT_DEATH(m(0, 4), "");
}

TYPED_TEST(Matrix4x4Test, operator_parenthesis_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_DEATH(M(4, 0), "");
  EXPECT_DEATH(M(0, 4), "");
}

TYPED_TEST(Matrix4x4Test, operator_subscript)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m[0] = 5;
  m[1] = 6;
  m[2] = 7;
  m[3] = 8;
  EXPECT_EQ(m[0], 5);
  EXPECT_EQ(m[1], 6);
  EXPECT_EQ(m[2], 7);
  EXPECT_EQ(m[3], 8);
}

TYPED_TEST(Matrix4x4Test, operator_subscript_const)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_EQ(M[0], 1);
  EXPECT_EQ(M[1], 2);
  EXPECT_EQ(M[2], 3);
  EXPECT_EQ(M[3], 4);
}

TYPED_TEST(Matrix4x4Test, operator_subscript_out_of_range)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m[0] = 5;
  EXPECT_DEATH(m[17], "");
}

TYPED_TEST(Matrix4x4Test, operator_subscript_const_out_of_range)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_DEATH(M[17], "");
}

TYPED_TEST(Matrix4x4Test, operator_plus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m += TypeParam{1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F,
                                        14.0F, 15.0F, 16.0F, 17.0F));
}

TYPED_TEST(Matrix4x4Test, operator_minus_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m -= TypeParam{1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F};
  EXPECT_THAT(m, ::testing::ElementsAre(0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                        13.0F, 14.0F, 15.0F));
}

TYPED_TEST(Matrix4x4Test, operator_multiply_equal)
{
  TypeParam m{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  m *= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F, 20.0F, 22.0F, 24.0F,
                                        26.0F, 28.0F, 30.0F, 32.0F));
}

TYPED_TEST(Matrix4x4Test, operator_divide_equal)
{
  TypeParam m{2.0F,  4.0F,  6.0F,  8.0F,  10.0F, 12.0F, 14.0F, 16.0F,
              18.0F, 20.0F, 22.0F, 24.0F, 26.0F, 28.0F, 30.0F, 32.0F};
  m /= 2;
  EXPECT_THAT(m, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F,
                                        13.0F, 14.0F, 15.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, operator_plus)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{5.0F,  6.0F,  7.0F,  8.0F,  9.0F,  10.0F, 11.0F, 12.0F,
                         13.0F, 14.0F, 15.0F, 16.0F, 17.0F, 18.0F, 19.0F, 20.0F};
  const auto m3 = M1 + M2;
  EXPECT_EQ(m3.rows(), 4);
  EXPECT_EQ(m3.cols(), 4);

  EXPECT_THAT(m3, ::testing::ElementsAre(6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F, 20.0F, 22.0F, 24.0F, 26.0F,
                                         28.0F, 30.0F, 32.0F, 34.0F, 36.0F));
}

TYPED_TEST(Matrix4x4Test, operator_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{5.0F,  6.0F,  7.0F,  8.0F,  9.0F,  10.0F, 11.0F, 12.0F,
                         13.0F, 14.0F, 15.0F, 16.0F, 17.0F, 18.0F, 19.0F, 20.0F};
  const auto m3 = M1 - M2;
  EXPECT_EQ(m3.rows(), 4);
  EXPECT_EQ(m3.cols(), 4);

  EXPECT_THAT(m3, ::testing::ElementsAre(-4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F, -4.0F,
                                         -4.0F, -4.0F, -4.0F, -4.0F, -4.0F));
}

TYPED_TEST(Matrix4x4Test, operator_multiply)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{5.0F,  6.0F,  7.0F,  8.0F,  9.0F,  10.0F, 11.0F, 12.0F,
                         13.0F, 14.0F, 15.0F, 16.0F, 17.0F, 18.0F, 19.0F, 20.0F};
  const auto m3 = M1 * M2;
  EXPECT_EQ(m3.rows(), 4);
  EXPECT_EQ(m3.cols(), 4);

  EXPECT_THAT(m3, ::testing::ElementsAre(130.0F, 140.0F, 150.0F, 160.0F, 306.0F, 332.0F, 358.0F, 384.0F, 482.0F, 524.0F,
                                         566.0F, 608.0F, 658.0F, 716.0F, 774.0F, 832.0F));
}

TYPED_TEST(Matrix4x4Test, operator_multiply_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  const auto m2 = M1 * 2;
  EXPECT_EQ(m2.rows(), 4);
  EXPECT_EQ(m2.cols(), 4);

  EXPECT_THAT(m2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F, 10.0F, 12.0F, 14.0F, 16.0F, 18.0F, 20.0F, 22.0F, 24.0F,
                                         26.0F, 28.0F, 30.0F, 32.0F));
}

TYPED_TEST(Matrix4x4Test, operator_divide_scalar)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  const auto m2 = M1 / 2;
  EXPECT_EQ(m2.rows(), 4);
  EXPECT_EQ(m2.cols(), 4);

  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F / 2.0F, 2.0F / 2.0F, 3.0F / 2.0F, 4.0F / 2.0F, 5.0F / 2.0F, 6.0F / 2.0F,
                                         7.0F / 2.0F, 8.0F / 2.0F, 9.0F / 2.0F, 10.0F / 2.0F, 11.0F / 2.0F,
                                         12.0F / 2.0F, 13.0F / 2.0F, 14.0F / 2.0F, 15.0F / 2.0F, 16.0F / 2.0F));
}

TYPED_TEST(Matrix4x4Test, operator_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  EXPECT_TRUE(M1 == M2);
}

TYPED_TEST(Matrix4x4Test, operator_not_equal)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  constexpr TypeParam M2{5.0F,  6.0F,  7.0F,  8.0F,  9.0F,  10.0F, 11.0F, 12.0F,
                         13.0F, 14.0F, 15.0F, 16.0F, 17.0F, 18.0F, 19.0F, 20.0F};
  EXPECT_TRUE(M1 != M2);
}

TYPED_TEST(Matrix4x4Test, identity)
{
  constexpr auto M = TypeParam::identity();
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
}

TYPED_TEST(Matrix4x4Test, transpose)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  const auto m2 = rtw::math::transpose(M1);
  EXPECT_THAT(m2, ::testing::ElementsAre(1.0F, 5.0F, 9.0F, 13.0F, 2.0F, 6.0F, 10.0F, 14.0F, 3.0F, 7.0F, 11.0F, 15.0F,
                                         4.0F, 8.0F, 12.0F, 16.0F));
}

TYPED_TEST(Matrix4x4Test, determinant)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  const auto d = rtw::math::determinant(M);
  EXPECT_EQ(d, 0);
}

TYPED_TEST(Matrix4x4Test, unary_minus)
{
  constexpr TypeParam M1{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                         9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  const auto m2 = -M1;
  EXPECT_THAT(m2, ::testing::ElementsAre(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16));
}

TYPED_TEST(Matrix4x4Test, minor)
{
  constexpr TypeParam M{1.0F, 2.0F,  3.0F,  4.0F,  5.0F,  6.0F,  7.0F,  8.0F,
                        9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F};
  auto m = M.minor(0, 0);
  EXPECT_THAT(m, ::testing::ElementsAre(6, 7, 8, 10, 11, 12, 14, 15, 16));

  m = M.minor(1, 1);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 3, 4, 9, 11, 12, 13, 15, 16));

  m = M.minor(2, 2);
  EXPECT_THAT(m, ::testing::ElementsAre(1, 2, 4, 5, 6, 8, 13, 14, 16));
}
//-----------------------------------------------------------------------------------------
using Matrix5x5F = rtw::math::Matrix<float, 5, 5>;
using Matrix5x5D = rtw::math::Matrix<double, 5, 5>;
using Matrix5x5Q16 = rtw::math::Matrix<rtw::fixed_point::FixedPoint16, 5, 5>;
using Matrix5x5Q32 = rtw::math::Matrix<rtw::fixed_point::FixedPoint32, 5, 5>;

template <typename T>
class Matrix5x5Test : public ::testing::Test
{};

using Matrix5x5Types = ::testing::Types<Matrix5x5F, Matrix5x5D, Matrix5x5Q16, Matrix5x5Q32>;
TYPED_TEST_SUITE(Matrix5x5Test, Matrix5x5Types, );

TYPED_TEST(Matrix5x5Test, inverse)
{
  constexpr auto IDENTITY = TypeParam::identity();

  constexpr TypeParam A{12.0F, -51.0F, 4.0F, 7.0F,  -2.0F, 6.0F,  167.0F, -68.0F, -3.0F, 5.0F,  -4.0F, 24.0F, -41.0F,
                        2.0F,  9.0F,   5.0F, -6.0F, 7.0F,  14.0F, -10.0F, -2.0F,  8.0F,  -3.0F, 11.0F, 6.0F};

  const auto identity_default = rtw::math::inverse(A) * A;
  const auto identity_hoseholder = rtw::math::matrix_decomposition::qr::householder::inverse(A) * A;
  const auto identity_givens = rtw::math::matrix_decomposition::qr::givens::inverse(A) * A;
  const auto identity_mgs = rtw::math::matrix_decomposition::qr::modified_gram_schmidt::inverse(A) * A;
  for (std::uint32_t i = 0U; i < IDENTITY.size(); ++i)
  {
    EXPECT_NEAR(static_cast<double>(identity_default[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
    EXPECT_NEAR(static_cast<double>(identity_hoseholder[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
    EXPECT_NEAR(static_cast<double>(identity_givens[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
    EXPECT_NEAR(static_cast<double>(identity_mgs[i]), static_cast<double>(IDENTITY[i]), EPSILON<TypeParam>);
  }
}
//-----------------------------------------------------------------------------------------
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
