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
