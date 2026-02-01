#include "math/matrix_eigen_decomposition.h"
#include "math/matrix_operations.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Test matrices were generated using https://catonmat.net/tools/generate-random-matrices
// and validated with https://matrixcalc.org/

template <typename T>
class EigenDecompositionTest : public ::testing::Test
{
public:
  template <typename MatrixT>
  void verify_eigen_decomposition(const MatrixT& a)
  {
    using MatrixValueType = typename MatrixT::value_type;
    using Matrix = MatrixT;
    using ComplexMatrix = rtw::math::Matrix<std::complex<MatrixValueType>, Matrix::NUM_ROWS, Matrix::NUM_COLS>;

    // The float calculations are not very precise, so we need a larger epsilon.
    constexpr static MatrixValueType EPSILON = std::is_same_v<MatrixValueType, float> ? 3e-3F : 1e-12;

    const auto eigenvalues = rtw::math::eigen_decomposition::eigenvalues(a);
    const auto eigenvectors = rtw::math::eigen_decomposition::eigenvectors(a, eigenvalues.eigenvalues);
    const auto inv_eigenvectors = rtw::math::inverse(eigenvectors);
    const auto diagonal = ComplexMatrix::diagonal(eigenvalues.eigenvalues);
    const auto reconstructed = eigenvectors * diagonal * inv_eigenvectors;

    for (std::uint32_t i = 0U; i < a.size(); ++i)
    {
      EXPECT_NEAR(a[i], reconstructed.template real<MatrixValueType>()[i], EPSILON);
    }
  }
};

using FloatingPointTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(EigenDecompositionTest, FloatingPointTypes, );

TYPED_TEST(EigenDecompositionTest, matrix3x3)
{
  using Matrix = rtw::math::Matrix<TypeParam, 3, 3>;

  constexpr Matrix A{
      // clang-format off
      -26.0F, -33.0F, -25.0F,
       31.0F,  42.0F,  23.0F,
      -11.0F, -15.0F,  -4.0F,
      // clang-format on
  };

  this->verify_eigen_decomposition(A);
}

TYPED_TEST(EigenDecompositionTest, matrix4x4)
{
  using Matrix = rtw::math::Matrix<TypeParam, 4, 4>;

  constexpr Matrix A{
      // clang-format off
       1.70239F, -3.79641F,  6.79370F, -9.67865F,
      -3.76763F, -3.79176F, -5.38801F,  6.05051F,
      -1.16207F,  3.42778F,  2.45114F,  9.81088F,
       1.69093F,  1.34002F,  2.29560F, -7.88903F,
      // clang-format on
  };

  this->verify_eigen_decomposition(A);
}

TYPED_TEST(EigenDecompositionTest, matrix5x5)
{
  using Matrix = rtw::math::Matrix<TypeParam, 5, 5>;

  constexpr Matrix A{
      // clang-format off
       1.22450F, -9.32449F,  8.85567F,  6.50187F,  2.27291F,
      -0.91175F, -2.68532F, -5.36380F,  0.08317F,  0.77245F,
       1.12802F,  0.27313F, -2.74689F,  8.16756F, -0.42767F,
       1.19478F,  6.22039F, -6.70452F, -4.83679F, -4.16758F,
      -0.75779F,  3.78038F,  6.19055F,  2.58007F,  0.48941F,
      // clang-format on
  };

  this->verify_eigen_decomposition(A);
}

TYPED_TEST(EigenDecompositionTest, matrix6x6)
{
  using Matrix = rtw::math::Matrix<TypeParam, 6, 6>;

  constexpr Matrix A{
      // clang-format off
      -0.96989F,  0.82089F, -4.06725F, -5.37137F, -4.55794F, -5.10598F,
      -4.28121F,  6.98662F, -3.26784F,  3.52817F, -4.96434F,  4.79570F,
       1.38155F, -2.55092F,  1.76686F, -3.67601F, -1.27047F,  2.65197F,
      -4.42689F,  9.71931F, -9.87039F, -7.82595F,  2.67180F, -2.44355F,
       2.76872F,  2.97529F,  4.02976F, -4.26067F, -4.92890F,  3.96606F,
      -2.52848F,  7.82574F, -0.78679F,  2.74249F, -0.02987F, -0.73763F,
      // clang-format on
  };

  this->verify_eigen_decomposition(A);
}

TYPED_TEST(EigenDecompositionTest, matrix10x10)
{
  using Matrix = rtw::math::Matrix<TypeParam, 10, 10>;

  constexpr Matrix A{
      // clang-format off
       2.38380F,  0.52448F, -9.16897F, -4.18491F,  8.08590F,  7.75736F,  7.52598F,  3.54309F, -4.34733F,  6.69285F,
      -6.39430F,  3.75281F, -6.47701F,  7.12694F,  8.92555F, -0.04259F, -3.65707F, -8.26348F, -4.32056F,  6.41108F,
      -1.98555F, -3.13857F,  2.38419F, -9.66639F, -5.84617F, -1.16749F,  4.71056F, -7.90592F,  0.57912F,  7.18285F,
      -8.78852F,  6.07515F, -8.36764F,  2.12867F, -4.25011F, -9.71815F,  3.67238F,  5.01201F, -4.88431F, -3.13733F,
       5.87131F,  6.54570F,  0.86283F, -4.99058F, -5.40110F, -3.79402F, -4.23029F,  9.67775F,  0.56833F, -6.72947F,
       0.39243F, -7.90772F,  6.47480F,  6.22138F,  1.22480F,  9.67023F,  1.79418F, -9.88004F, -3.99513F,  0.29977F,
      -6.98051F, -0.67262F,  7.61430F,  3.95406F,  9.86012F,  6.96295F,  8.35955F, -6.16348F,  2.90020F,  0.25019F,
      -7.10155F, -3.06247F,  8.78803F, -2.49253F,  9.11246F, -5.04579F, -2.88409F, -0.79928F,  1.01932F, -4.83908F,
      -5.07066F, -7.58619F,  4.66267F, -3.23968F, -5.19784F,  0.87096F,  2.86131F, -3.04916F,  5.65880F, -7.13709F,
      -3.17953F,  3.69229F,  1.33019F,  4.40403F, -4.04221F,  4.02688F,  7.73380F, -2.80557F,  5.68021F, -0.39696F,
      // clang-format on
  };

  this->verify_eigen_decomposition(A);
}
