#pragma once

#include "fixed_point/fixed_point.h"
#include "fixed_point/math.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <type_traits>

namespace rtw::math
{

struct UninitializedTag
{
  enum class Tag : std::uint8_t
  {
    TAG
  };
  constexpr explicit UninitializedTag(Tag /*tag*/) noexcept {}
};
constexpr UninitializedTag UNINITIALIZED{UninitializedTag::Tag::TAG};

struct ZeroTag
{
  enum class Tag : std::uint8_t
  {
    TAG
  };
  constexpr explicit ZeroTag(Tag /*tag*/) noexcept {}
};
constexpr ZeroTag ZERO{ZeroTag::Tag::TAG};

struct IdentityTag
{
  enum class Tag : std::uint8_t
  {
    TAG
  };
  constexpr explicit IdentityTag(Tag /*tag*/) noexcept {}
};
constexpr IdentityTag IDENTITY{IdentityTag::Tag::TAG};

/// A matrix that is statically sized.
/// The matrix is stored in column-major order.
/// @tparam T The type of the elements.
/// @tparam ROWS The number of rows.
/// @tparam COLS The number of columns.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
class Matrix
{
  constexpr static std::uint32_t SIZE = ROWS * COLS;

  template <std::uint16_t ROW_COUNT>
  constexpr static bool HAS_X = (ROW_COUNT > 0);

  template <std::uint16_t ROW_COUNT>
  constexpr static bool HAS_Y = (ROW_COUNT > 1);

  template <std::uint16_t ROW_COUNT>
  constexpr static bool HAS_Z = (ROW_COUNT > 2);

  template <std::uint16_t ROW_COUNT>
  constexpr static std::uint16_t W_INDEX = (ROW_COUNT > 3 ? 3 : 2);

  template <typename U = T, std::uint16_t OTHER_ROWS, std::uint16_t OTHER_COLS, std::uint16_t... INDEX>
  constexpr static std::array<T, SIZE> from_matrix(const Matrix<U, OTHER_ROWS, OTHER_COLS>& matrix,
                                                   std::integer_sequence<std::uint16_t, INDEX...> /*index*/) noexcept
  {
    static_assert((OTHER_ROWS * OTHER_COLS) <= SIZE, "Too many elements");
    return std::array<T, SIZE>{matrix[INDEX]...};
  }

  template <std::uint16_t... INDEX>
  constexpr static std::array<T, SIZE> make_identity(std::integer_sequence<std::uint16_t, INDEX...> /*index*/) noexcept
  {
    return std::array<T, SIZE>{(INDEX % (ROWS + 1) == 0 ? T{1} : T{0})...};
  }

public:
  using value_type = T;
  using iterator = typename std::array<value_type, SIZE>::iterator;
  using const_iterator = typename std::array<value_type, SIZE>::const_iterator;
  using pointer = typename std::array<value_type, SIZE>::pointer;
  using const_pointer = typename std::array<value_type, SIZE>::const_pointer;
  using reference = typename std::array<value_type, SIZE>::reference;

  static_assert(ROWS > 0, "ROWS must be greater than 0");
  static_assert(COLS > 0, "COLS must be greater than 0");
  static_assert(fixed_point::IS_ARITHMETIC_V<T>, "T must be arithmetic");

  constexpr explicit Matrix(UninitializedTag /*tag*/) noexcept {}
  constexpr explicit Matrix(ZeroTag /*tag*/) noexcept : data_{value_type{0}} {}
  constexpr explicit Matrix(IdentityTag /*tag*/) noexcept
      : data_{make_identity(std::make_integer_sequence<std::uint16_t, SIZE>{})}
  {
  }
  constexpr Matrix() noexcept : Matrix{math::ZERO} {}

  template <typename... ArgsT,
            typename = std::enable_if_t<
                (sizeof...(ArgsT) <= SIZE)
                && (fixed_point::IS_ARITHMETIC_V<std::remove_cv_t<std::remove_reference_t<ArgsT>>> && ...)>>
  constexpr explicit Matrix(ArgsT&&... args) noexcept : data_{std::forward<ArgsT>(args)...}
  {
  }

  template <typename U = value_type, std::uint16_t OTHER_ROWS, std::uint16_t OTHER_COLS,
            typename = std::enable_if_t<std::is_same_v<U, value_type> && (OTHER_ROWS <= ROWS) && (OTHER_COLS <= COLS)>>
  constexpr explicit Matrix(const Matrix<U, OTHER_ROWS, OTHER_COLS>& matrix) noexcept
      : data_{from_matrix(matrix, std::make_integer_sequence<std::uint16_t, OTHER_ROWS * OTHER_COLS>{})}
  {
  }

  constexpr std::uint16_t rows() const noexcept { return ROWS; }
  constexpr std::uint16_t cols() const noexcept { return COLS; }
  constexpr std::uint32_t size() const noexcept { return SIZE; }

  template <typename U = value_type, typename = std::enable_if_t<fixed_point::IS_ARITHMETIC_V<U>>>
  constexpr Matrix<U, ROWS, COLS> cast() const noexcept
  {
    Matrix<U, ROWS, COLS> result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = static_cast<U>(data_[i]);
    }
    return result;
  }

  constexpr reference operator()(const std::uint16_t row, const std::uint16_t col) noexcept
  {
    assert(row < ROWS);
    assert(col < COLS);
    return data_[row * COLS + col];
  }
  constexpr value_type operator()(const std::uint16_t row, const std::uint16_t col) const noexcept
  {
    assert(row < ROWS);
    assert(col < COLS);
    return data_[row * COLS + col];
  }

  constexpr reference operator[](const std::uint32_t index) noexcept
  {
    assert(index < SIZE);
    return data_[index];
  }
  constexpr value_type operator[](const std::uint32_t index) const noexcept
  {
    assert(index < SIZE);
    return data_[index];
  }

  /// Accessors for the first column.
  /// @{
  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_X<ROW_COUNT>>>
  constexpr value_type x() const noexcept
  {
    return operator[](0);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_X<ROW_COUNT>>>
  constexpr reference x() noexcept
  {
    return operator[](0);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Y<ROW_COUNT>>>
  constexpr value_type y() const noexcept
  {
    return operator[](1);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Y<ROW_COUNT>>>
  constexpr reference y() noexcept
  {
    return operator[](1);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Z<ROW_COUNT>>>
  constexpr value_type z() const noexcept
  {
    return operator[](2);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Z<ROW_COUNT>>>
  constexpr reference z() noexcept
  {
    return operator[](2);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Z<ROW_COUNT>>>
  constexpr value_type w() const noexcept
  {
    return operator[](W_INDEX<ROW_COUNT>);
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<HAS_Z<ROW_COUNT>>>
  constexpr reference w() noexcept
  {
    return operator[](W_INDEX<ROW_COUNT>);
  }
  /// @}

  constexpr Matrix& operator+=(const Matrix& rhs) noexcept
  {
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      data_[i] += rhs[i];
    }
    return *this;
  }

  constexpr Matrix& operator-=(const Matrix& rhs) noexcept
  {
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      data_[i] -= rhs[i];
    }
    return *this;
  }

  constexpr Matrix& operator/=(const value_type rhs) noexcept
  {
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      data_[i] /= rhs;
    }
    return *this;
  }

  constexpr Matrix& operator*=(const value_type rhs) noexcept
  {
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      data_[i] *= rhs;
    }
    return *this;
  }

  /// Swizzle operators.
  /// @{
  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<(ROW_COUNT > 2)>>
  constexpr Matrix<value_type, 2, 1> xy() const noexcept
  {
    return Matrix<value_type, 2, 1>{x(), y()};
  }

  template <std::uint16_t ROW_COUNT = ROWS, typename = std::enable_if_t<(ROW_COUNT > 3)>>
  constexpr Matrix<value_type, 3, 1> xyz() const noexcept
  {
    return Matrix<value_type, 3, 1>{x(), y(), z()};
  }
  /// @}

  constexpr Matrix<T, ROWS, 1> column(const std::uint16_t col) const noexcept
  {
    assert(col < COLS);
    Matrix<T, ROWS, 1> result{math::UNINITIALIZED};
    for (std::uint16_t row = 0U; row < ROWS; ++row)
    {
      result[row] = (*this)(row, col);
    }
    return result;
  }

  constexpr Matrix<T, COLS, 1> row(const std::uint16_t row) const noexcept
  {
    assert(row < ROWS);
    Matrix<T, COLS, 1> result{math::UNINITIALIZED};
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      result[col] = (*this)(row, col);
    }
    return result;
  }

  template <std::uint16_t ROW_COUNT = ROWS, std::uint16_t COL_COUNT = COLS,
            typename = std::enable_if_t<(ROW_COUNT == COL_COUNT) && (ROW_COUNT > 2) && (COL_COUNT > 2)>>
  constexpr Matrix<T, ROW_COUNT - 1, COL_COUNT - 1> minor(const std::uint16_t row,
                                                          const std::uint16_t col) const noexcept
  {
    assert(row < ROW_COUNT);
    assert(col < COL_COUNT);
    Matrix<T, ROW_COUNT - 1, COL_COUNT - 1> result{math::UNINITIALIZED};
    for (std::uint16_t r = 0U; r < ROW_COUNT - 1; ++r)
    {
      const std::uint16_t rr = r < row ? r : r + 1;
      for (std::uint16_t c = 0U; c < COL_COUNT - 1; ++c)
      {
        const std::uint16_t cc = c < col ? c : c + 1;
        result(r, c) = (*this)(rr, cc);
      }
    }
    return result;
  }

  constexpr iterator begin() noexcept { return data_.begin(); }
  constexpr const_iterator begin() const noexcept { return data_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }

  constexpr iterator end() noexcept { return data_.end(); }
  constexpr const_iterator end() const noexcept { return data_.end(); }
  constexpr const_iterator cend() const noexcept { return data_.cend(); }

  constexpr pointer data() noexcept { return data_.data(); }
  constexpr const_pointer data() const noexcept { return data_.data(); }

  constexpr static Matrix identity() noexcept { return Matrix{math::IDENTITY}; }
  constexpr static Matrix zero() noexcept { return Matrix{math::ZERO}; }

  std::ostream& operator<<(std::ostream& os) const noexcept
  {
    os << std::fixed << std::setprecision(4) << '[';
    if constexpr (COLS == 1)
    {
      for (std::uint16_t row = 0U; row < ROWS; ++row)
      {
        os << (*this)(row, 0);
        if (row < ROWS - 1U)
        {
          os << ' ';
        }
      }
    }
    else
    {
      os << '\n';
      for (std::uint16_t row = 0U; row < ROWS; ++row)
      {
        for (std::uint16_t col = 0U; col < COLS; ++col)
        {
          os << std::setw(10) << (*this)(row, col);
          if (col < COLS - 1U)
          {
            os << ' ';
          }
        }
        os << '\n';
      }
    }
    os << ']';
    return os;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Matrix operator+(const Matrix& lhs, const Matrix& rhs) noexcept
  {
    Matrix result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = lhs[i] + rhs[i];
    }
    return result;
  }

  friend constexpr Matrix operator-(const Matrix& lhs, const Matrix& rhs) noexcept
  {
    Matrix result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = lhs[i] - rhs[i];
    }
    return result;
  }

  friend constexpr Matrix operator*(const Matrix& lhs, const value_type rhs) noexcept
  {
    Matrix result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = lhs[i] * rhs;
    }
    return result;
  }

  friend constexpr Matrix operator*(const value_type lhs, const Matrix& rhs) noexcept { return rhs * lhs; }

  friend constexpr Matrix operator/(const Matrix& lhs, const value_type rhs) noexcept
  {
    Matrix result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = lhs[i] / rhs;
    }
    return result;
  }

  template <std::uint16_t OTHER_COLS>
  friend constexpr Matrix<value_type, ROWS, OTHER_COLS>
  operator*(const Matrix& lhs, const Matrix<value_type, COLS, OTHER_COLS>& rhs) noexcept
  {
    // TODO: make this more cache friendly.
    Matrix<value_type, ROWS, OTHER_COLS> result{math::ZERO};
    for (std::uint16_t row = 0U; row < ROWS; ++row)
    {
      for (std::uint16_t other_col = 0U; other_col < OTHER_COLS; ++other_col)
      {
        for (std::uint16_t col = 0U; col < COLS; ++col)
        {
          result(row, other_col) += lhs(row, col) * rhs(col, other_col);
        }
      }
    }
    return result;
  }

  template <std::uint16_t OTHER_COLS>
  friend constexpr Matrix<value_type, ROWS, OTHER_COLS> operator*(const Matrix<value_type, ROWS, COLS>& lhs,
                                                                  const Matrix& rhs) noexcept
  {
    return rhs * lhs;
  }

  friend constexpr Matrix operator-(const Matrix& rhs) noexcept
  {
    Matrix result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = -rhs[i];
    }
    return result;
  }

  friend constexpr bool operator==(const Matrix& lhs, const Matrix& rhs) noexcept
  {
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      if (lhs[i] != rhs[i])
      {
        return false;
      }
    }
    return true;
  }

  friend constexpr bool operator!=(const Matrix& lhs, const Matrix& rhs) noexcept { return !(lhs == rhs); }

  friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix) noexcept
  {
    os << "Matrix" << ROWS << "x" << COLS;
    return matrix.operator<<(os);
  }
  /// @}

private:
  std::array<value_type, SIZE> data_;
};

/// Compute the transpose of a matrix.
/// @param[in] matrix The matrix.
/// @return The transpose of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
constexpr Matrix<T, COLS, ROWS> transpose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Matrix<T, COLS, ROWS> result{math::UNINITIALIZED};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    for (std::uint16_t col = 0U; col < COLS; ++col)
    {
      result(col, row) = matrix(row, col);
    }
  }
  return result;
}

template <typename T>
using Matrix2x2 = Matrix<T, 2, 2>;
using Matrix2x2f = Matrix2x2<float>;
using Matrix2x2d = Matrix2x2<double>;
using Matrix2x2q16 = Matrix2x2<fixed_point::FixedPoint16>;
using Matrix2x2q32 = Matrix2x2<fixed_point::FixedPoint32>;

template <typename T>
using Matrix3x3 = Matrix<T, 3, 3>;
using Matrix3x3f = Matrix3x3<float>;
using Matrix3x3d = Matrix3x3<double>;
using Matrix3x3q16 = Matrix3x3<fixed_point::FixedPoint16>;
using Matrix3x3q32 = Matrix3x3<fixed_point::FixedPoint32>;

template <typename T>
using Matrix4x4 = Matrix<T, 4, 4>;
using Matrix4x4f = Matrix4x4<float>;
using Matrix4x4d = Matrix4x4<double>;
using Matrix4x4q16 = Matrix4x4<fixed_point::FixedPoint16>;
using Matrix4x4q32 = Matrix4x4<fixed_point::FixedPoint32>;

/// Compute the determinant of a square, 2 by 2 matrix.
/// https://en.m.wikipedia.org/wiki/Determinant
/// @param[in] matrix The matrix.
/// @return The determinant of the matrix.
template <typename T>
constexpr T determinant(const Matrix2x2<T>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(1, 0);
  const auto d = matrix(1, 1);
  return a * d - b * c;
}

/// Compute the determinant of a square, 3 by 3 matrix.
/// https://en.m.wikipedia.org/wiki/Determinant
/// @param[in] matrix The matrix.
/// @return The determinant of the matrix.
template <typename T>
constexpr T determinant(const Matrix3x3<T>& matrix) noexcept
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
  return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS == COLS) && (ROWS > 3)>>
constexpr T determinant(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  T det{0};
  constexpr std::int8_t SIGNS[] = {1, -1};
  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    const auto sign = SIGNS[col % 2]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    det += sign * matrix(0, col) * determinant(matrix.minor(0, col));
  }
  return det;
}

/// Compute the inverse of a square, 2 by 2 matrix.
/// https://en.m.wikipedia.org/wiki/Invertible_matrix#Inversion_of_2_%C3%97_2_matrices
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix2x2<T> inverse(const Matrix2x2<T>& matrix) noexcept
{
  const auto a = matrix(0, 0);
  const auto b = matrix(0, 1);
  const auto c = matrix(1, 0);
  const auto d = matrix(1, 1);
  const auto det = determinant(matrix);
  assert(det != T{0});
  const auto inv_det = T{1} / det;
  return inv_det * Matrix2x2<T>{d, -b, -c, a};
}

/// Compute the inverse of a square, 3 by 3 matrix.
/// https://en.m.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3_%C3%97_3_matrices
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T>
constexpr Matrix3x3<T> inverse(const Matrix3x3<T>& matrix) noexcept
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
  const auto res_a = e * i - f * h;
  const auto res_b = -(d * i - f * g);
  const auto res_c = d * h - e * g;
  const auto res_d = -(b * i - c * h);
  const auto res_e = a * i - c * g;
  const auto res_f = -(a * h - b * g);
  const auto res_g = b * f - c * e;
  const auto res_h = -(a * f - c * d);
  const auto res_i = a * e - b * d;
  const auto det = determinant(matrix);
  assert(det != T{0});
  const auto inv_det = T{1} / det;
  return inv_det * Matrix3x3<T>{res_a, res_d, res_g, res_b, res_e, res_h, res_c, res_f, res_i};
}

template <typename T, std::uint16_t ROWS>
constexpr T dot(const Matrix<T, ROWS, 1>& lhs, const Matrix<T, ROWS, 1>& rhs) noexcept
{
  T result{0};
  for (std::uint16_t row = 0U; row < ROWS; ++row)
  {
    result += lhs[row] * rhs[row];
  }
  return result;
}

template <typename T, std::uint16_t ROWS>
constexpr T norm2(const Matrix<T, ROWS, 1>& vector) noexcept
{
  return dot(vector, vector);
}

template <typename T, std::uint16_t ROWS>
constexpr T norm(const Matrix<T, ROWS, 1>& vector) noexcept
{
  using fixed_point::math::sqrt;
  using std::sqrt;
  return sqrt(norm2(vector));
}

template <typename T, std::uint16_t ROWS>
constexpr Matrix<T, ROWS, 1> normalize(const Matrix<T, ROWS, 1>& vector) noexcept
{
  const auto n = norm(vector);
  assert(n != T{0});
  return vector / n;
}

namespace householder::qr
{

namespace details
{

/// Compute the Householder vector for a column of a matrix.
/// @param[in] m The matrix.
/// @param[in] col The column.
/// @return The Householder vector.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> get_householder_vector(const Matrix<T, ROWS, COLS>& m, const std::uint16_t col) noexcept
{
  Matrix<T, ROWS, 1> v = m.column(col);

  // Zero the elements below the diagonal to create a Householder vector.
  for (std::uint16_t c = 0U; c < col; ++c)
  {
    v[c] = T{0};
  }

  constexpr std::int8_t SIGNS[] = {1, -1};
  const auto sign = SIGNS[v[col] < 0];
  const auto alpha = sign * norm(v); // Adjust the sign of alpha to prevent numerical instability.
  v[col] += alpha;                   // v[k] = v[k] + sign(v[k]) * ||v||

  return normalize(v);
}

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
  for (std::uint16_t row = ROWS - 1U; row < ROWS; --row)
  {
    result[row] = vector[row];
    for (std::uint16_t col = row + 1U; col < COLS; ++col)
    {
      result[row] -= matrix(row, col) * result[col];
    }
    result[row] /= matrix(row, row);
  }
  return result;
}

} // namespace details

template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
struct Decomposition
{
  Matrix<T, ROWS, ROWS> q{math::UNINITIALIZED};
  Matrix<T, ROWS, COLS> r{math::UNINITIALIZED};
};

/// Compute the QR decomposition of a matrix using Householder reflections.
/// https://en.wikipedia.org/wiki/QR_decomposition#Using_Householder_reflections
/// @param[in] matrix The matrix.
/// @param[out] q The orthogonal matrix. The q matrix is nor transposed.
/// @param[out] r The upper triangular matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Decomposition<T, ROWS, COLS> decompose(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  Decomposition<T, ROWS, COLS> result{Matrix<T, ROWS, COLS>{math::IDENTITY}, matrix};

  // The number of columns is reduced by one if the matrix is square.
  // The last column is not processed, because it is already upper triangular.
  const auto cols = COLS - (ROWS == COLS);
  for (std::uint16_t col = 0U; col < cols; ++col)
  {
    const Matrix<T, ROWS, 1> v = details::get_householder_vector(result.r, col);

    const auto vv_t_doubled = T{2} * v * transpose(v);
    result.r -= vv_t_doubled * result.r;
    result.q -= vv_t_doubled * result.q;
  }

  return result;
}

/// Compute the inverse of a matrix using the Householder QR decomposition.
/// @param[in] matrix The matrix.
/// @return The inverse of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto [q, r] = decompose(matrix);
  const auto r_inv = details::inverse_upper_triangular(r);
  return r_inv * q;
}

/// Solve a system of linear equations (Ax = b) using the Householder QR decomposition.
/// @param[in] a The matrix.
/// @param[in] b The vector.
/// @return The solution to the system of linear equations.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, 1> solve(const Matrix<T, ROWS, COLS>& a, const Matrix<T, ROWS, 1>& b) noexcept
{
  const auto [q, r] = decompose(a);
  return details::back_substitution(r, q * b);
}

} // namespace householder::qr

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS) && (ROWS > 3)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  return householder::qr::inverse(matrix);
}

} // namespace rtw::math
