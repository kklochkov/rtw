#pragma once

#include "math/matrix_decomposition.h"
#include "math/numeric.h"

#include <complex>

namespace rtw::math::eigen_decomposition
{

namespace qr
{

namespace details
{

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
struct HessenbergResult
{
  Matrix<T, ROWS, COLS> h{math::UNINITIALIZED};
  Matrix<T, ROWS, COLS> q{math::UNINITIALIZED};
};

/// Compute the Hessenberg form of a matrix using Householder reflections.
/// https://en.wikipedia.org/wiki/Hessenberg_matrix#Reduction_to_Hessenberg_matrix
/// @param[in] matrix The matrix.
/// @return The Hessenberg form of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr HessenbergResult<T, ROWS, COLS> hessenberg(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  using namespace matrix_decomposition::qr;

  HessenbergResult<T, ROWS, COLS> result{matrix, Matrix<T, ROWS, COLS>{math::IDENTITY}};

  for (std::uint16_t col = 0U; col < COLS - 2U; ++col)
  {
    const auto maybe_h = householder::details::get_householder_matrix(result.h, col, col + 1U);

    if (!maybe_h.has_value())
    {
      continue; // Skip if the Householder matrix is not defined (denominator is zero).
    }

    const auto h = maybe_h.value();
    result.h = h * result.h * h; // A = H * A * H
    result.q = result.q * h;

    {
      // Zero out the elements below the sub-diagonal to form the Hessenberg matrix.
      for (std::uint16_t row = col + 2U; row < ROWS; ++row)
      {
        result.h(row, col) = T{0};
      }
    }
  }

  return result;
}

template <typename T>
struct TraceResult
{
  T trace{0};
  T determinant{1};
};

/// Compute the trace and determinant of a 2x2 sub-matrix starting at (diagonal_index, diagonal_index).
/// @param[in] matrix The matrix.
/// @param[in] diagonal_index The index of the diagonal element to start from.
/// @return The trace and determinant of the 2x2 sub-matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr TraceResult<T> trace_determinant_2x2(const Matrix<T, ROWS, COLS>& matrix,
                                               const std::size_t diagonal_index) noexcept
{
  TraceResult<T> result;
  const auto a = matrix(diagonal_index, diagonal_index);
  const auto b = matrix(diagonal_index, diagonal_index + 1U);
  const auto c = matrix(diagonal_index + 1U, diagonal_index);
  const auto d = matrix(diagonal_index + 1U, diagonal_index + 1U);
  result.trace = a + d;
  result.determinant = (a * d) - (b * c); // This is 2d cross product or perpendicular product.
  return result;
}

template <typename T, std::uint16_t ROWS>
constexpr T partial_norm(const Matrix<T, ROWS, 1U>& v, const std::uint16_t rows) noexcept
{
  // Create a MatrixView or VectorView and implement some methods there?
  using multiprecision::math::sqrt;
  using std::sqrt;

  T norm{0};
  for (std::uint16_t row = 0U; row < rows; ++row)
  {
    norm += v[row] * v[row];
  }
  return sqrt(norm);
}

template <typename T, std::uint16_t ROWS>
constexpr void inplace_partial_normalize(Matrix<T, ROWS, 1U>& v, const std::uint16_t rows) noexcept
{
  // Create a MatrixView or VectorView and implement some methods there?
  auto norm = partial_norm(v, rows);
  if (norm == T{0})
  {
    return;
  }

  norm = T{1} / norm; // Normalize the vector.
  for (std::uint16_t row = 0U; row < rows; ++row)
  {
    v[row] *= norm;
  }
}

template <typename T, std::uint16_t ROWS>
constexpr void inplace_normalize_householder_vector(Matrix<T, ROWS, 1U>& v, const std::uint16_t rows) noexcept
{
  const std::uint16_t start_row = 0U;
  const auto sign = multiprecision::sign(v[start_row] < 0);
  const auto norm = partial_norm(v, rows);
  const auto alpha = sign * norm; // Adjust the sign of norm to prevent numerical instability.
  v[start_row] += alpha;          // v[start_row] = v[start_row] + sign(v[start_row]) * ||v||

  inplace_partial_normalize(v, rows);
}

/// Apply the Householder transformation H = I - 2 * v * v^T to the left of the matrix.
/// @param[in,out] matrix The matrix to apply the transformation to.
/// @param[in] v The Householder vector.
/// @param[in] start_row The starting row of the sub-matrix to apply the transformation to.
/// @param[in] start_col The starting column of the sub-matrix to apply the transformation to.
/// @param[in] end_col The ending column of the sub-matrix to apply the transformation to.
/// @param[in] sub_rows The number of rows in the Householder vector to use.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, std::uint16_t V_ROWS,
          typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr void inplace_apply_householder_vector_left(Matrix<T, ROWS, COLS>& matrix, const Matrix<T, V_ROWS, 1U>& v,
                                                     const std::uint16_t start_row, const std::uint16_t start_col,
                                                     const std::uint16_t end_col, const std::uint16_t sub_rows) noexcept
{
  for (std::uint16_t sub_col = start_col; sub_col <= end_col; ++sub_col)
  {
    T dot{0};
    for (std::uint16_t sub_row = 0U; sub_row < sub_rows; ++sub_row)
    {
      dot += v[sub_row] * matrix(start_row + sub_row, sub_col);
    }
    const auto scaled_dot = T{2} * dot;
    for (std::uint16_t sub_row = 0U; sub_row < sub_rows; ++sub_row)
    {
      matrix(start_row + sub_row, sub_col) -= v[sub_row] * scaled_dot;
    }
  }
}

/// Apply the Householder transformation H = I - 2 * v * v^T to the right of the matrix.
/// @param[in,out] matrix The matrix to apply the transformation to.
/// @param[in] v The Householder vector.
/// @param[in] start_col The starting column of the sub-matrix to apply the transformation to.
/// @param[in] start_row The starting row of the sub-matrix to apply the transformation to.
/// @param[in] end_row The ending row of the sub-matrix to apply the transformation to.
/// @param[in] sub_cols The number of columns in the Householder vector to use.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, std::uint16_t V_ROWS,
          typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr void inplace_apply_householder_vector_right(Matrix<T, ROWS, COLS>& matrix, const Matrix<T, V_ROWS, 1U>& v,
                                                      const std::uint16_t start_col, const std::uint16_t start_row,
                                                      const std::uint16_t end_row,
                                                      const std::uint16_t sub_cols) noexcept
{
  for (std::uint16_t sub_row = start_row; sub_row <= end_row; ++sub_row)
  {
    T dot{0};
    for (std::uint16_t sub_col = 0U; sub_col < sub_cols; ++sub_col)
    {
      dot += v[sub_col] * matrix(sub_row, start_col + sub_col);
    }
    const auto scaled_dot = T{2} * dot;
    for (std::uint16_t sub_col = 0U; sub_col < sub_cols; ++sub_col)
    {
      matrix(sub_row, start_col + sub_col) -= v[sub_col] * scaled_dot;
    }
  }
}

/// Perform an in-place Francis step on a Hessenberg matrix.
/// https://en.wikipedia.org/wiki/QR_algorithm
/// @param[in,out] matrix The Hessenberg matrix to perform the Francis step on.
/// @param[in] n The size of the matrix (number of rows or columns).
/// Note: The matrix must be at least 3x3.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr void inplace_francis_step(Matrix<T, ROWS, COLS>& matrix, const std::uint16_t n) noexcept
{
  // 1. Compute trace and determinant of the 2x2 block (Wilkinson polynomial).
  const auto td = trace_determinant_2x2(matrix, n - 2U);

  // 2. Build Francois 3-element vector.
  Matrix<T, 3U, 1U> francis_v{math::UNINITIALIZED};
  francis_v[0U] =
      matrix(0U, 0U) * matrix(0U, 0U) + matrix(0U, 1U) * matrix(1U, 0U) - td.trace * matrix(0U, 0U) + td.determinant;
  francis_v[1U] = matrix(1U, 0U) * (matrix(0U, 0U) + matrix(1U, 1U) - td.trace);
  francis_v[2U] = matrix(1U, 0U) * matrix(2U, 1U);

  // 3. Bulge-chasing step.
  std::uint16_t m = 3U; // the Francois vector size.
  for (std::uint16_t row = 0U; row < n - 1U; ++row)
  {
    inplace_normalize_householder_vector(francis_v, m);

    const std::uint16_t start_col = (row != 0U) * (row - 1U);
    inplace_apply_householder_vector_left(matrix, francis_v, row, start_col, n - 1U, m);

    const std::uint16_t start_row = std::min(row + 3U, n - 1U);
    inplace_apply_householder_vector_right(matrix, francis_v, row, 0U, start_row, m);

    if ((row + 2U) == n)
    {
      break;
    }

    francis_v[0U] = matrix(row + 1U, row);
    francis_v[1U] = matrix(row + 2U, row);
    if ((row + 3U) < n)
    {
      francis_v[2U] = matrix(row + 3U, row);
      m = 3U;
    }
    else
    {
      francis_v[2U] = T{0}; // Zero out the last element if it is not needed.
      m = 2U;
    }
  }
}

/// Extract the eigenvalues from a quasi-upper triangular matrix.
/// @param[in] matrix The quasi-upper triangular matrix.
/// @param[in] tolerance The tolerance for determining if an element is zero.
/// @return The eigenvalues of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename ComplexT = std::complex<T>,
          typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<ComplexT, ROWS, 1U> extract_eigenvalues(const Matrix<T, ROWS, COLS>& matrix,
                                                         const T tolerance) noexcept
{
  using multiprecision::math::abs;
  using multiprecision::math::sqrt;
  using std::abs;
  using std::sqrt;

  Matrix<ComplexT, ROWS, 1U> result{math::ZERO};

  for (std::uint16_t row = 0U; row < ROWS;)
  {
    if ((row < (ROWS - 1U)) && (abs(matrix(row + 1U, row)) > tolerance))
    {
      // 2x2 block
      const auto trace_det = trace_determinant_2x2(matrix, row);
      const auto half_trace = trace_det.trace * T{0.5};
      const auto discriminant = (half_trace * half_trace) - trace_det.determinant;
      const auto sqrt_discriminant = sqrt(abs(discriminant));
      if (discriminant < T{0})
      {
        // Complex eigenvalues, extract real part only.
        result[row] = ComplexT{half_trace, sqrt_discriminant};
        result[row + 1U] = ComplexT{half_trace, -sqrt_discriminant};
      }
      else
      {
        // Real eigenvalues
        const auto sign = multiprecision::sign(half_trace < 0);
        const auto eigenvalue1 = half_trace + (sign * sqrt_discriminant);
        const auto eigenvalue2 = trace_det.determinant / eigenvalue1;
        result[row] = ComplexT{eigenvalue1, T{0}};
        result[row + 1U] = ComplexT{eigenvalue2, T{0}};
      }
      row += 2U;
    }
    else
    {
      // 1x1 block
      result[row] = matrix(row, row);
      ++row;
    }
  }

  return result;
}

/// Check if a matrix is a diagonal matrix.
/// @param[in] matrix The matrix to check.
/// @param[in] tolerance The tolerance for determining if an element is zero.
/// @return True if the matrix is diagonal, false otherwise.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr bool is_diagonal(const Matrix<T, ROWS, COLS>& matrix, const T tolerance) noexcept
{
  using multiprecision::math::abs;
  using std::abs;

  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      if (row != col)
      {
        if constexpr (multiprecision::IS_COMPLEX_V<T>)
        {
          if (abs(matrix(row, col)) > tolerance.real())
          {
            return false;
          }
        }
        else
        {
          if (abs(matrix(row, col)) > tolerance)
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}

} // namespace details

template <typename T, std::size_t ROWS, typename ComplexT = std::complex<T>>
struct EigenvaluesResult
{
  Matrix<ComplexT, ROWS, 1U> eigenvalues{math::ZERO};
  std::size_t iterations{0U};
};

/// Compute the eigenvalues of a matrix using the QR algorithm with Francis steps.
/// The algorithm first reduces the matrix to Hessenberg form, then iteratively applies
/// Francis steps to converge to a quasi-upper triangular form.
/// @param[in] matrix The matrix.
/// @param[in] max_iterations The maximum number of iterations to perform.
/// @param[in] tolerance The tolerance for convergence.
/// @return The eigenvalues of the matrix and the number of iterations performed.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr EigenvaluesResult<T, ROWS> eigenvalues(const Matrix<T, ROWS, COLS>& matrix,
                                                 const std::uint16_t max_iterations = 1'000U,
                                                 const T tolerance = default_tolerance<T>()) noexcept
{
  using multiprecision::math::abs;
  using std::abs;

  auto hessenberg = details::hessenberg(matrix);
  auto& h = hessenberg.h;

  EigenvaluesResult<T, ROWS> result{};

  std::uint16_t n = ROWS;
  for (result.iterations = 0U; result.iterations < max_iterations; ++result.iterations)
  {
    {
      // 1. Process the sub-diagonal elements.
      for (std::uint16_t row = 0U; row < n - 1U; ++row)
      {
        if (abs(h(row + 1U, row)) <= (tolerance * (abs(h(row, row)) + abs(h(row + 1U, row + 1U)))))
        {
          // Zero out the sub-diagonal element.
          h(row + 1U, row) = T{0};
        }
      }
    }

    {
      // 2. Deflate the sub-diagonal elements.
      while (n >= 3U)
      {
        if (h(n - 1U, n - 2U) == T{0})
        {
          --n; // 1x1 block separated.
        }
        else if (h(n - 2U, n - 3U) == T{0})
        {
          n -= 2U; // 2x2 block separated.
        }
        else
        {
          break; // No more deflation possible.
        }
      }

      if (n < 3U)
      {
        break; // No more sub-diagonal elements to process.
      }
    }

    // 3. Apply the Francois step to the last 3x3 block.
    details::inplace_francis_step(h, n);
  }

  result.eigenvalues = details::extract_eigenvalues(h, tolerance);

  return result;
}

/// Compute the eigenvectors of a matrix given its eigenvalues using the Householder QR decomposition.
/// The algorithm solves the equation (A - λI)v = 0 for each eigenvalue λ to find the corresponding eigenvector v.
/// The eigenvectors are normalized to have unit length.
/// If the matrix is diagonal, the eigenvectors are the standard basis vectors.
/// @param[in] matrix The matrix.
/// @param[in] eigenvalues The eigenvalues of the matrix.
/// @return The eigenvectors of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                             const Matrix<T, ROWS, 1U>& eigenvalues,
                                             const T tolerance = default_tolerance<T>()) noexcept
{
  using namespace matrix_decomposition::qr;

  const Matrix<T, ROWS, COLS> identity{math::IDENTITY};
  Matrix<T, ROWS, COLS> result{math::UNINITIALIZED};

  if (details::is_diagonal(matrix, tolerance))
  {
    // If the matrix is diagonal, the eigenvectors are the standard basis vectors.
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      for (std::uint16_t row = 0U; row < ROWS; ++row)
      {
        result(row, col) = (row == col) ? T{1} : T{0};
      }
    }
    return result;
  }

  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    const auto a_minus_lambda_i = matrix - (eigenvalues[col] * identity);
    Matrix<T, ROWS, 1U> unit_vector{math::ZERO};
    unit_vector[col] = T{1};

    auto nullspace_vector = householder::solve(a_minus_lambda_i, unit_vector);
    nullspace_vector = normalize(nullspace_vector);

    for (std::uint16_t row = 0U; row < ROWS; ++row)
    {
      result(row, col) = nullspace_vector[row];
    }
  }

  return result;
}

} // namespace qr

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr eigen_decomposition::qr::EigenvaluesResult<T, ROWS>
eigenvalues(const Matrix<T, ROWS, COLS>& matrix, const std::uint16_t max_iterations = 1'000U,
            const T tolerance = default_tolerance<T>()) noexcept
{
  return eigen_decomposition::qr::eigenvalues(matrix, max_iterations, tolerance);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                             const Matrix<T, ROWS, 1U>& eigenvalues,
                                             const T tolerance = default_tolerance<T>()) noexcept
{
  return eigen_decomposition::qr::eigenvectors(matrix, eigenvalues, tolerance);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename ComplexT = std::complex<T>,
          typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<ComplexT, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                                    const Matrix<ComplexT, ROWS, 1U>& eigenvalues,
                                                    const ComplexT tolerance = default_tolerance<ComplexT>()) noexcept
{
  return eigen_decomposition::qr::eigenvectors(matrix.template cast<ComplexT>(), eigenvalues, tolerance);
}

} // namespace rtw::math::eigen_decomposition
