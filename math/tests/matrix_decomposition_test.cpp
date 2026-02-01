#include "math/matrix.h"
#include "math/matrix_decomposition.h"
#include "math/matrix_operations.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Test matrices were generated using https://catonmat.net/tools/generate-random-matrices
// and validated with https://matrixcalc.org/

// FixedPoint16 has lower precision and requires a larger epsilon
template <typename T>
constexpr auto EPSILON = std::is_same_v<typename T::value_type, rtw::multiprecision::FixedPoint16> ? 7.0e-3F : 1.0e-4F;

template <typename T>
class Matrix2x2Test : public ::testing::Test
{};

using Matrix2x2Types =
    ::testing::Types<rtw::math::Matrix2x2F, rtw::math::Matrix2x2D, rtw::math::Matrix2x2Q16, rtw::math::Matrix2x2Q32>;
TYPED_TEST_SUITE(Matrix2x2Test, Matrix2x2Types, );

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

TYPED_TEST(Matrix3x3Test, cholesky_decomposition)
{
  constexpr TypeParam EXPECTED_A{0.5428F, 0.6227F, 0.8200F, 0.6227F, 1.7851F, 1.0809F, 0.8200F, 1.0809F, 1.3183F};
  const auto l = rtw::math::matrix_decomposition::cholesky::decompose(EXPECTED_A);
  const auto lt = rtw::math::transpose(l);
  const auto a = l * lt;
  for (std::uint32_t i = 0U; i < a.size(); ++i)
  {
    EXPECT_NEAR(static_cast<double>(a[i]), static_cast<double>(EXPECTED_A[i]), EPSILON<TypeParam>);
  }
}

TYPED_TEST(Matrix3x3Test, cholesky_solve)
{
  using Vector = rtw::math::Matrix<typename TypeParam::value_type, 3, 1>;
  constexpr TypeParam A{0.5428F, 0.6227F, 0.8200F, 0.6227F, 1.7851F, 1.0809F, 0.8200F, 1.0809F, 1.3183F};
  constexpr Vector B{1.0F, 2.0F, 3.0F};

  const auto x = rtw::math::matrix_decomposition::qr::modified_gram_schmidt::solve(A, B);
  const auto ax = A * x;
  for (std::uint32_t i = 0U; i < ax.size(); ++i)
  {
    EXPECT_NEAR(static_cast<double>(ax[i]), static_cast<double>(B[i]), EPSILON<TypeParam>);
  }
}
//-----------------------------------------------------------------------------------------
using Matrix5x5F = rtw::math::Matrix<float, 5, 5>;
using Matrix5x5D = rtw::math::Matrix<double, 5, 5>;
using Matrix5x5Q16 = rtw::math::Matrix<rtw::multiprecision::FixedPoint16, 5, 5>;
using Matrix5x5Q32 = rtw::math::Matrix<rtw::multiprecision::FixedPoint32, 5, 5>;

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
