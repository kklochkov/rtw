#pragma once

#include "math/matrix.h"
#include "math/matrix_decomposition.h"

namespace rtw::math
{

/// Compute the transpose of a matrix.
/// @param[in] matrix The matrix.
/// @return The transpose of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, MemoryOrder MEMORY_ORDER>
constexpr Matrix<T, COLS, ROWS, MEMORY_ORDER> transpose(const Matrix<T, ROWS, COLS, MEMORY_ORDER>& matrix) noexcept
{
  Matrix<T, COLS, ROWS, MEMORY_ORDER> result{math::UNINITIALIZED};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      result(col, row) = matrix(row, col);
    }
  }
  return result;
}

/// Compute the determinant of a square, 2 by 2 matrix.
/// https://en.m.wikipedia.org/wiki/Determinant
/// @param[in] matrix The matrix.
/// @return The determinant of the matrix.
template <typename T, MemoryOrder MEMORY_ORDER>
constexpr T determinant(const Matrix2x2<T, MEMORY_ORDER>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(1, 0);
  const auto d = matrix(1, 1);
  return (a * d) - (b * c);
}

/// Compute the determinant of a square, 3 by 3 matrix.
/// https://en.m.wikipedia.org/wiki/Determinant
/// @param[in] matrix The matrix.
/// @return The determinant of the matrix.
template <typename T, MemoryOrder MEMORY_ORDER>
constexpr T determinant(const Matrix3x3<T, MEMORY_ORDER>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(0, 2);
  const auto d = matrix(1, 0);
  const auto e = matrix(1, 1);
  const auto f = matrix(1, 2);
  const auto g = matrix(2, 0);
  const auto h = matrix(2, 1);
  const auto i = matrix(2, 2);
  return (a * e * i) + (b * f * g) + (c * d * h) - (c * e * g) - (b * d * i) - (a * f * h);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, MemoryOrder MEMORY_ORDER,
          typename = std::enable_if_t<(ROWS == COLS) && (ROWS > 3)>>
constexpr T determinant(const Matrix<T, ROWS, COLS, MEMORY_ORDER>& matrix) noexcept
{
  T det{0};
  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    const auto sign = multiprecision::sign(col % 2);
    det += sign * matrix(0, col) * determinant(matrix.minor(0, col));
  }
  return det;
}

/// Compute the inverse of a square, 2 by 2 matrix.
/// https://en.m.wikipedia.org/wiki/Invertible_matrix#Inversion_of_2_%C3%97_2_matrices
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, MemoryOrder MEMORY_ORDER>
constexpr Matrix2x2<T, MEMORY_ORDER> inverse(const Matrix2x2<T, MEMORY_ORDER>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(1, 0);
  const auto d = matrix(1, 1);
  const auto det = determinant(matrix);
  assert(det != T{0});
  const auto inv_det = T{1} / det;
  // clang-format off
  return inv_det * Matrix2x2<T, MEMORY_ORDER>{FROM_ROW_MAJOR,
    d, -b,
    -c, a,
  };
  // clang-format on
}

/// Compute the inverse of a square, 3 by 3 matrix.
/// https://en.m.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_%C3%97_3_matrices
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, MemoryOrder MEMORY_ORDER>
constexpr Matrix3x3<T, MEMORY_ORDER> inverse(const Matrix3x3<T, MEMORY_ORDER>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(0, 2);
  const auto d = matrix(1, 0);
  const auto e = matrix(1, 1);
  const auto f = matrix(1, 2);
  const auto g = matrix(2, 0);
  const auto h = matrix(2, 1);
  const auto i = matrix(2, 2);
  const auto res_a = (e * i) - (f * h);
  const auto res_b = -((d * i) - (f * g));
  const auto res_c = (d * h) - (e * g);
  const auto res_d = -((b * i) - (c * h));
  const auto res_e = (a * i) - (c * g);
  const auto res_f = -((a * h) - (b * g));
  const auto res_g = (b * f) - (c * e);
  const auto res_h = -((a * f) - (c * d));
  const auto res_i = (a * e) - (b * d);
  const auto det = determinant(matrix);
  assert(det != T{0});
  const auto inv_det = T{1} / det;
  // clang-format off
  return inv_det * Matrix3x3<T, MEMORY_ORDER>{FROM_ROW_MAJOR,
    res_a, res_d, res_g,
    res_b, res_e, res_h,
    res_c, res_f, res_i,
  };
  // clang-format on
}

/// Compute the inverse of a square, 4x4 matrix using Cramer's rule.
/// This is more efficient than QR decomposition for 4x4 matrices.
/// https://en.wikipedia.org/wiki/Invertible_matrix#Analytic_solution
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, MemoryOrder MEMORY_ORDER>
constexpr Matrix4x4<T, MEMORY_ORDER> inverse(const Matrix4x4<T, MEMORY_ORDER>& matrix) noexcept
{
  const auto m00 = matrix(0, 0);
  const auto m01 = matrix(0, 1);
  const auto m02 = matrix(0, 2);
  const auto m03 = matrix(0, 3);
  const auto m10 = matrix(1, 0);
  const auto m11 = matrix(1, 1);
  const auto m12 = matrix(1, 2);
  const auto m13 = matrix(1, 3);
  const auto m20 = matrix(2, 0);
  const auto m21 = matrix(2, 1);
  const auto m22 = matrix(2, 2);
  const auto m23 = matrix(2, 3);
  const auto m30 = matrix(3, 0);
  const auto m31 = matrix(3, 1);
  const auto m32 = matrix(3, 2);
  const auto m33 = matrix(3, 3);

  // Compute 2x2 sub-determinants
  const auto s0 = (m00 * m11) - (m10 * m01);
  const auto s1 = (m00 * m12) - (m10 * m02);
  const auto s2 = (m00 * m13) - (m10 * m03);
  const auto s3 = (m01 * m12) - (m11 * m02);
  const auto s4 = (m01 * m13) - (m11 * m03);
  const auto s5 = (m02 * m13) - (m12 * m03);
  const auto c5 = (m22 * m33) - (m32 * m23);
  const auto c4 = (m21 * m33) - (m31 * m23);
  const auto c3 = (m21 * m32) - (m31 * m22);
  const auto c2 = (m20 * m33) - (m30 * m23);
  const auto c1 = (m20 * m32) - (m30 * m22);
  const auto c0 = (m20 * m31) - (m30 * m21);

  // Compute determinant
  const auto det = (s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) - (s4 * c1) + (s5 * c0);
  assert(det != T{0});
  const auto inv_det = T{1} / det;
  // clang-format off
  return inv_det * Matrix4x4<T, MEMORY_ORDER>{math::FROM_ROW_MAJOR,
    ( m11 * c5) - (m12 * c4) + (m13 * c3), (-m01 * c5) + (m02 * c4) - (m03 * c3),
    ( m31 * s5) - (m32 * s4) + (m33 * s3), (-m21 * s5) + (m22 * s4) - (m23 * s3),

    (-m10 * c5) + (m12 * c2) - (m13 * c1), ( m00 * c5) - (m02 * c2) + (m03 * c1),
    (-m30 * s5) + (m32 * s2) - (m33 * s1), ( m20 * s5) - (m22 * s2) + (m23 * s1),

    ( m10 * c4) - (m11 * c2) + (m13 * c0), (-m00 * c4) + (m01 * c2) - (m03 * c0),
    ( m30 * s4) - (m31 * s2) + (m33 * s0), (-m20 * s4) + (m21 * s2) - (m23 * s0),

    (-m10 * c3) + (m11 * c1) - (m12 * c0), ( m00 * c3) - (m01 * c1) + (m02 * c0),
    (-m30 * s3) + (m31 * s1) - (m32 * s0), ( m20 * s3) - (m21 * s1) + (m22 * s0),
  };
  // clang-format on
}

template <typename T, std::uint16_t ROWS, MemoryOrder MEMORY_ORDER>
constexpr T dot(const Matrix<T, ROWS, 1, MEMORY_ORDER>& lhs, const Matrix<T, ROWS, 1, MEMORY_ORDER>& rhs) noexcept
{
  T result{0};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    result += lhs[row] * rhs[row];
  }
  return result;
}

template <typename T, std::uint16_t ROWS, MemoryOrder MEMORY_ORDER>
constexpr T norm2(const Matrix<T, ROWS, 1, MEMORY_ORDER>& vector) noexcept
{
  return dot(vector, vector);
}

template <typename T, std::uint16_t ROWS, MemoryOrder MEMORY_ORDER>
constexpr T norm(const Matrix<T, ROWS, 1, MEMORY_ORDER>& vector) noexcept
{
  using multiprecision::math::sqrt;
  using std::sqrt;
  return sqrt(norm2(vector));
}

template <typename T, std::uint16_t ROWS, MemoryOrder MEMORY_ORDER>
constexpr Matrix<T, ROWS, 1, MEMORY_ORDER> normalize(const Matrix<T, ROWS, 1, MEMORY_ORDER>& vector) noexcept
{
  const auto n = norm(vector);
  assert(n != T{0});
  return vector / n;
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, MemoryOrder MEMORY_ORDER,
          typename = std::enable_if_t<(ROWS >= COLS) && (ROWS > 3)>>
constexpr Matrix<T, ROWS, COLS, MEMORY_ORDER> inverse(const Matrix<T, ROWS, COLS, MEMORY_ORDER>& matrix) noexcept
{
  return matrix_decomposition::qr::householder::inverse(matrix);
}

} // namespace rtw::math
