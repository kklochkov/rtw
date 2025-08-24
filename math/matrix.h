#pragma once

#include "fixed_point/fixed_point.h"
#include "fixed_point/math.h"

#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace rtw::math
{

struct UninitializedTag
{
  // clang-format off: https://github.com/llvm/llvm-project/issues/62067
  enum class Tag : std::uint8_t { TAG };
  // clang-format on
  constexpr explicit UninitializedTag(Tag /*tag*/) noexcept {}
};
constexpr inline UninitializedTag UNINITIALIZED{UninitializedTag::Tag::TAG};

struct InitializeWithValueTag
{
  // clang-format off
  enum class Tag : std::uint8_t { TAG };
  // clang-format on
  constexpr explicit InitializeWithValueTag(Tag /*tag*/) noexcept {}
};

constexpr inline InitializeWithValueTag INITIALIZE_WITH_VALUE{InitializeWithValueTag::Tag::TAG};

struct ZeroTag
{
  // clang-format off
  enum class Tag : std::uint8_t { TAG };
  // clang-format on
  constexpr explicit ZeroTag(Tag /*tag*/) noexcept {}
};
constexpr inline ZeroTag ZERO{ZeroTag::Tag::TAG};

struct IdentityTag
{
  // clang-format off
  enum class Tag : std::uint8_t { TAG };
  // clang-format on
  constexpr explicit IdentityTag(Tag /*tag*/) noexcept {}
};
constexpr inline IdentityTag IDENTITY{IdentityTag::Tag::TAG};

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
  constexpr static std::array<T, SIZE> make_with_diagonal(std::integer_sequence<std::uint16_t, INDEX...> /*index*/,
                                                          const T value) noexcept
  {
    return std::array<T, SIZE>{(INDEX % (ROWS + 1) == 0 ? value : T{0})...};
  }

  template <std::uint16_t... INDEX>
  constexpr static std::array<T, SIZE> make_with_value(std::integer_sequence<std::uint16_t, INDEX...> /*index*/,
                                                       const T value) noexcept
  {
    return std::array<T, SIZE>{(
        [&value]()
        {
          (void)INDEX;
          return value;
        }())...};
  }

public:
  using value_type = T;
  using iterator = typename std::array<value_type, SIZE>::iterator;
  using const_iterator = typename std::array<value_type, SIZE>::const_iterator;
  using pointer = typename std::array<value_type, SIZE>::pointer;
  using const_pointer = typename std::array<value_type, SIZE>::const_pointer;
  using reference = typename std::array<value_type, SIZE>::reference;

  constexpr static std::uint16_t NUM_ROWS = ROWS;
  constexpr static std::uint16_t NUM_COLS = COLS;

  static_assert(ROWS > 0, "ROWS must be greater than 0");
  static_assert(COLS > 0, "COLS must be greater than 0");
  static_assert(fixed_point::IS_ARITHMETIC_V<T>, "T must be arithmetic");

  constexpr explicit Matrix(UninitializedTag /*tag*/) noexcept {}
  constexpr explicit Matrix(ZeroTag /*tag*/) noexcept : data_{value_type{0}} {}
  constexpr explicit Matrix(IdentityTag /*tag*/) noexcept
      : data_{make_with_diagonal(std::make_integer_sequence<std::uint16_t, SIZE>{}, T{1})}
  {
  }
  constexpr explicit Matrix(InitializeWithValueTag /*tag*/, const T value) noexcept
      : data_{make_with_value(std::make_integer_sequence<std::uint16_t, SIZE>{}, value)}
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

  template <
      typename U = value_type, std::uint16_t OTHER_ROWS, std::uint16_t OTHER_COLS,
      typename = std::enable_if_t<std::is_convertible_v<U, value_type> && (OTHER_ROWS <= ROWS) && (OTHER_COLS <= COLS)>>
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

  template <typename U = value_type,
            typename = std::enable_if_t<fixed_point::IS_ARITHMETIC_V<U> && fixed_point::IS_COMPLEX_V<T>>>
  constexpr Matrix<U, ROWS, COLS> real() const noexcept
  {
    Matrix<U, ROWS, COLS> result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = static_cast<U>(data_[i].real());
    }
    return result;
  }

  constexpr reference operator()(const std::uint16_t row, const std::uint16_t col) noexcept
  {
    assert(row < ROWS);
    assert(col < COLS);
    return data_[(row * COLS) + col];
  }
  constexpr value_type operator()(const std::uint16_t row, const std::uint16_t col) const noexcept
  {
    assert(row < ROWS);
    assert(col < COLS);
    return data_[(row * COLS) + col];
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
    assert(rhs != 0);
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
  constexpr static Matrix diagonal(const Matrix<T, ROWS, 1>& diag) noexcept
  {
    Matrix result{math::ZERO};
    for (std::uint16_t i = 0U; i < ROWS; ++i)
    {
      result(i, i) = diag[i];
    }
    return result;
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
    assert(rhs != T{0});
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
using Matrix2x2F = Matrix2x2<float>;
using Matrix2x2D = Matrix2x2<double>;
using Matrix2x2Q16 = Matrix2x2<fixed_point::FixedPoint16>;
using Matrix2x2Q32 = Matrix2x2<fixed_point::FixedPoint32>;

template <typename T>
using Matrix3x3 = Matrix<T, 3, 3>;
using Matrix3x3F = Matrix3x3<float>;
using Matrix3x3D = Matrix3x3<double>;
using Matrix3x3Q16 = Matrix3x3<fixed_point::FixedPoint16>;
using Matrix3x3Q32 = Matrix3x3<fixed_point::FixedPoint32>;

template <typename T>
using Matrix4x4 = Matrix<T, 4, 4>;
using Matrix4x4F = Matrix4x4<float>;
using Matrix4x4D = Matrix4x4<double>;
using Matrix4x4Q16 = Matrix4x4<fixed_point::FixedPoint16>;
using Matrix4x4Q32 = Matrix4x4<fixed_point::FixedPoint32>;

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
  return (a * d) - (b * c);
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
  return (a * e * i) + (b * f * g) + (c * d * h) - (c * e * g) - (b * d * i) - (a * f * h);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS == COLS) && (ROWS > 3)>>
constexpr T determinant(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  T det{0};
  for (std::uint16_t col = 0U; col < COLS; ++col)
  {
    const auto sign = fixed_point::sign(col % 2);
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

namespace matrix_decomposition
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

  using fixed_point::math::sqrt;
  using std::sqrt;
  norm = sqrt(norm);

  T sign{1};
  if constexpr (fixed_point::IS_COMPLEX_V<T>)
  {
    sign = fixed_point::sign(v[start_row].real() < 0);
  }
  else
  {
    sign = fixed_point::sign(v[start_row] < 0);
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
  using fixed_point::math::sqrt;
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
} // namespace matrix_decomposition

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS) && (ROWS > 3)>>
constexpr Matrix<T, ROWS, COLS> inverse(const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  return matrix_decomposition::qr::householder::inverse(matrix);
}

namespace eigen_decomposition::qr
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
  using fixed_point::math::sqrt;
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
  auto norm = partial_norm(v, rows);
  if (norm == T{0})
  {
    return;
  }

  const std::uint16_t start_row = 0U;
  const auto sign = fixed_point::sign(v[start_row] < 0);
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
  using fixed_point::math::abs;
  using fixed_point::math::sqrt;
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
        const auto sign = fixed_point::sign(half_trace < 0);
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

} // namespace details

/// Default tolerance for convergence in the eigenvalue algorithm.
/// The default tolerance is set to 100 times the machine epsilon for the floating-point type T,
/// for fixed-point types it is set to 1, since the epsilon for fixed-point types is exactly 1.
/// @tparam T The floating-point or fixed-point type.
/// @return The default tolerance.
template <typename T>
constexpr T default_tolerance() noexcept
{
  std::uint16_t tolerance_factor = 100U;
  if constexpr (fixed_point::IS_FIXED_POINT_V<T>)
  {
    tolerance_factor = 1;
  }

  return tolerance_factor * std::numeric_limits<T>::epsilon();
}

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
  using fixed_point::math::abs;
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
/// @param[in] matrix The matrix.
/// @param[in] eigenvalues The eigenvalues of the matrix.
/// @return The eigenvectors of the matrix.
template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                             const Matrix<T, ROWS, 1U>& eigenvalues) noexcept
{
  using namespace matrix_decomposition::qr;

  const Matrix<T, ROWS, COLS> identity{math::IDENTITY};
  Matrix<T, ROWS, COLS> result{math::UNINITIALIZED};

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

} // namespace eigen_decomposition::qr

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr eigen_decomposition::qr::EigenvaluesResult<T, ROWS>
eigenvalues(const Matrix<T, ROWS, COLS>& matrix, const std::uint16_t max_iterations = 1'000U,
            const T tolerance = eigen_decomposition::qr::default_tolerance<T>()) noexcept
{
  return eigen_decomposition::qr::eigenvalues(matrix, max_iterations, tolerance);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<T, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                             const Matrix<T, ROWS, 1U>& eigenvalues) noexcept
{
  return eigen_decomposition::qr::eigenvectors(matrix, eigenvalues);
}

template <typename T, std::uint16_t ROWS, std::uint16_t COLS, typename ComplexT = std::complex<T>,
          typename = std::enable_if_t<(ROWS >= COLS)>>
constexpr Matrix<ComplexT, ROWS, COLS> eigenvectors(const Matrix<T, ROWS, COLS>& matrix,
                                                    const Matrix<ComplexT, ROWS, 1U>& eigenvalues) noexcept
{
  return eigen_decomposition::qr::eigenvectors(matrix.template cast<ComplexT>(), eigenvalues);
}

} // namespace rtw::math
