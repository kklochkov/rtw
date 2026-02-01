#pragma once

#include "math/matrix.h"
#include "multiprecision/math.h"

namespace rtw::math::matrix_decomposition
{

/// Compute the inverse of an upper triangular matrix.
/// https://en.wikipedia.org/wiki/Triangular_matrix#Inversion
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> inverse_upper_triangular(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Matrix<T, ROWS, COLS> result{math::IDENTITY};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      for (std::uint16_t i = 0U; i < row; ++i)
      {
        result(col, row) -= matrix(i, row) * result(col, i);
      }
      assert(matrix(row, row) != T{0});
      result(col, row) /= matrix(row, row);
    }
  }
  return result;
}

/// Compute the back substitution of a matrix of a system of linear equations.
/// https://en.wikipedia.org/wiki/Triangular_matrix#Forward_and_back_substitution
/// @param[in] matrix The matrix. The matrix must be upper triangular.
/// @param[in] vector The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> back_substitution(const Matrix<T, ROWS, COLS>& matrix,
                                               const Matrix<T, ROWS, 1>& vector) noexcept
{
  Matrix<T, ROWS, 1> result{math::UNINITIALIZED};
  for (std::uint16_t row = ROWS; row-- > 0U;)
  {
    result[row] = vector[row];
    for (std::uint16_t col = row + 1U; col < COLS; ++col)
    {
      result[row] -= matrix(row, col) * result[col];
    }
    assert(matrix(row, row) != T{0});
    result[row] /= matrix(row, row);
  }
  return result;
}

/// Compute the forward substitution of a matrix of a system of linear equations.
/// https://en.wikipedia.org/wiki/Triangular_matrix#Forward_and_back_substitution
/// @param[in] matrix The matrix. The matrix must be lower triangular.
/// @param[in] vector The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> forward_substitution(const Matrix<T, ROWS, COLS>& matrix,
                                                  const Matrix<T, ROWS, 1>& vector) noexcept
{
  Matrix<T, ROWS, 1> result{math::UNINITIALIZED};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    result[row] = vector[row];
    for (std::uint16_t col = 0U; col < row; ++col)
    {
      result[row] -= matrix(row, col) * result[col];
    }
    assert(matrix(row, row) != T{0});
    result[row] /= matrix(row, row);
  }
  return result;
}

namespace qr
{

template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
struct Decomposition
{
  Matrix<T, ROWS, ROWS> q{math::UNINITIALIZED};
  Matrix<T, ROWS, COLS> r{math::UNINITIALIZED};
};

namespace householder
{

namespace details
{

/// Compute the Householder vector for a column of a matrix.
/// The vector is not normalized and is used in the QR decomposition where tau is computed as tau = 2 / (v^T * v)
/// for the Householder transformation H = I - tau * v * v^T.
/// @param[in] matrix The matrix.
/// @param[in] matrix_col The column of the matrix.
/// @param[in] start_row The starting row of the Householder vector.
/// @param[in] end_row The ending row of the Householder vector.
/// @return The Householder vector.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> get_householder_vector(const Matrix<T, ROWS, COLS>& matrix, const std::uint16_t matrix_col,
                                                    const std::uint16_t start_row) noexcept
{
  Matrix<T, ROWS, 1U> v{math::ZERO};
  for (std::uint16_t row = start_row; row < ROWS; ++row)
  {
    v[row] = matrix(row, matrix_col);
  }

  T norm{0};
  for (std::uint16_t row = start_row; row < ROWS; ++row)
  {
    norm += v[row] * v[row];
  }

  using multiprecision::math::sqrt;
  using std::sqrt;
  norm = sqrt(norm);

  T sign{1};
  if constexpr (multiprecision::IS_COMPLEX_V<T>)
  {
    sign = multiprecision::sign(v[start_row].real() < 0);
  }
  else
  {
    sign = multiprecision::sign(v[start_row] < 0);
  }

  const auto alpha = sign * norm; // Adjust the sign of norm to prevent numerical instability.
  v[start_row] += alpha;          // v[start_row] = v[start_row] + sign(v[start_row]) * ||v||

  if (v[start_row] != T{0})
  {
    // Normalize the Householder vector.
    for (std::uint16_t row = start_row + 1U; row < ROWS; ++row)
    {
      v[row] /= v[start_row];
    }
    v[start_row] = T{1};
  }

  return v;
}

/// Compute the Householder matrix for a column of a matrix.
/// H = I - tau * v * v^T, where tau = 2 / (v^T * v).
/// @param[in] matrix The matrix.
/// @param[in] matrix_col The column of the matrix.
/// @param[in] start_row The starting row of the Householder vector.
/// @return The Householder matrix, or std::nullopt if the denominator is zero.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr std::optional<Matrix<T, ROWS, COLS>> get_householder_matrix(const Matrix<T, ROWS, COLS>& matrix,
                                                                      const std::uint16_t matrix_col,
                                                                      const std::uint16_t start_row) noexcept
{
  const auto v = get_householder_vector(matrix, matrix_col, start_row);
  const auto vt = transpose(v);
  const Matrix<T, 1U, 1U> denominator = vt * v;

  if (denominator[0U] != T{0})
  {
    const auto beta = T{2} / denominator[0U];
    return Matrix<T, ROWS, COLS>{math::IDENTITY} - (beta * v * vt);
  }

  return std::nullopt;
}

} // namespace details

/// Compute the QR decomposition of a matrix using Householder reflections.
/// https://en.wikipedia.org/wiki/QR_decomposition#Using_Householder_reflections
/// @param[in] matrix The matrix.
/// @return The QR decomposition of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Decomposition<T, ROWS, COLS> decompose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Decomposition<T, ROWS, COLS> result{Matrix<T, ROWS, COLS>{math::IDENTITY}, matrix};

  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    const auto maybe_h = details::get_householder_matrix(result.r, col, col);
    if (!maybe_h.has_value())
    {
      continue; // Skip if the Householder matrix is not defined (denominator is zero).
    }

    const auto h = maybe_h.value();
    result.r = h * result.r;
    result.q = result.q * h;
  }

  result.q = transpose(result.q);
  return result;
}

/// Compute the inverse of a matrix using the Householder QR decomposition.
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto [q, r] = decompose(matrix);
  const auto r_inv = inverse_upper_triangular(r);
  return r_inv * q;
}

/// Solve a system of linear equations (Ax = b) using the Householder QR
/// decomposition.
/// @param[in] a The matrix.
/// @param[in] b The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> solve(const Matrix<T, ROWS, COLS>& a, const Matrix<T, ROWS, 1>& b) noexcept
{
  const auto [q, r] = decompose(a);
  return back_substitution(r, q * b);
}

} // namespace householder

namespace givens
{

namespace details
{

template <typename T>
constexpr T hypot(const T a, const T b) noexcept
{
  using multiprecision::math::sqrt;
  using std::sqrt;
  return sqrt((a * a) + (b * b));
}

} // namespace details

/// Compute the QR decomposition of a matrix using Givens rotations.
/// https://en.wikipedia.org/wiki/QR_decomposition#Using_Givens_rotations
/// @param[in] matrix The matrix.
/// @return The QR decomposition of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Decomposition<T, ROWS, COLS> decompose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Decomposition<T, ROWS, COLS> result{Matrix<T, ROWS, ROWS>{math::IDENTITY}, matrix};

  for (std::uint16_t col = 0U; col < COLS - 1U; ++col)
  {
    for (std::uint16_t row = col + 1U; row < ROWS; ++row)
    {
      const auto a = result.r(col, col);
      const auto b = result.r(row, col);
      const auto r = details::hypot(a, b);

      assert(r != T{0});

      const auto c = a / r;
      const auto s = -b / r;

      Matrix<T, ROWS, ROWS> g{math::IDENTITY};
      g(col, col) = c;
      g(row, row) = c;
      g(col, row) = -s;
      g(row, col) = s;

      result.r = g * result.r;
      result.q = result.q * transpose(g);
    }
  }

  result.q = transpose(result.q);
  return result;
}

/// Compute the inverse of a matrix using the Givens QR decomposition.
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto [q, r] = decompose(matrix);
  const auto r_inv = inverse_upper_triangular(r);
  return r_inv * q;
}

/// Solve a system of linear equations (Ax = b) using the Givens QR
/// decomposition.
/// @param[in] a The matrix.
/// @param[in] b The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> solve(const Matrix<T, ROWS, COLS>& a, const Matrix<T, ROWS, 1>& b) noexcept
{
  const auto [q, r] = decompose(a);
  return back_substitution(r, q * b);
}

} // namespace givens

namespace modified_gram_schmidt
{

/// Compute the QR decomposition of a matrix using the Modified Gram-Schmidt
/// algorithm.
/// https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process#Numerical_stability
/// @param[in] matrix The matrix.
/// @return The QR decomposition of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Decomposition<T, ROWS, COLS> decompose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Decomposition<T, ROWS, COLS> result{Matrix<T, ROWS, ROWS>{math::ZERO}, Matrix<T, ROWS, ROWS>{math::ZERO}};

  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    Matrix<T, ROWS, 1> v = matrix.column(col);

    // Orthogonalisation step
    {
      for (std::uint16_t c = 0U; c < col; ++c)
      {
        T dot{0};
        for (std::uint16_t row = 0U; row < ROWS; ++row)
        {
          dot += result.q(row, c) * v[row];
        }
        result.r(c, col) = dot;
        for (std::uint16_t row = 0U; row < ROWS; ++row)
        {
          v[row] -= dot * result.q(row, c);
        }
      }
    }

    // Normalisation step
    {
      result.r(col, col) = norm(v);

      assert(result.r(col, col) != T{0});

      for (std::uint16_t row = 0U; row < ROWS; ++row)
      {
        result.q(row, col) = v[row] / result.r(col, col);
      }
    }
  }

  result.q = transpose(result.q);
  return result;
}

/// Compute the inverse of a matrix using the Modified Gram-Schmidt QR
/// decomposition.
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto [q, r] = decompose(matrix);
  const auto r_inv = inverse_upper_triangular(r);
  return r_inv * q;
}

/// Solve a system of linear equations (Ax = b) using the Givens QR
/// decomposition.
/// @param[in] a The matrix.
/// @param[in] b The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> solve(const Matrix<T, ROWS, COLS>& a, const Matrix<T, ROWS, 1>& b) noexcept
{
  const auto [q, r] = decompose(a);
  return back_substitution(r, q * b);
}

} // namespace modified_gram_schmidt

} // namespace qr

namespace cholesky
{
/// Compute the Cholesky decomposition of a symmetric, positive-definite matrix.
/// https://en.wikipedia.org/wiki/Cholesky_decomposition
/// @param[in] matrix The matrix.
/// @return The Cholesky decomposition of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> decompose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  using multiprecision::math::sqrt;
  using std::sqrt;

  Matrix<T, ROWS, COLS> result{math::ZERO};

  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    for (std::uint16_t row = 0U; row <= col; ++row)
    {
      T sum{0};
      for (std::uint16_t k = 0U; k < row; ++k)
      {
        sum += result(row, k) * result(col, k);
      }

      if (row == col)
      {
        const auto diag = matrix(row, row) - sum;
        assert(diag > T{0}); // The matrix must be positive-definite.
        result(row, row) = sqrt(diag);
      }
      else
      {
        assert(result(row, row) != T{0});
        result(col, row) = (matrix(col, row) - sum) / result(row, row);
      }
    }
  }

  return result;
}

/// Solve a system of linear equations (Ax = b) using the Cholesky decomposition.
/// The matrix A must be symmetric and positive-definite.
/// @param[in] a The matrix.
/// @param[in] b The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> solve(const Matrix<T, ROWS, COLS>& a, const Matrix<T, ROWS, 1>& b) noexcept
{
  const auto l = decompose(a);
  const auto y = forward_substitution(l, b);
  return back_substitution(transpose(l), y);
}

} // namespace cholesky

} // namespace rtw::math::matrix_decomposition
