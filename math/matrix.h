#pragma once

#include "multiprecision/fixed_point.h"

#include <array>
#include <cassert>
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
  static_assert(multiprecision::IS_ARITHMETIC_V<T>, "T must be arithmetic");

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
                && (multiprecision::IS_ARITHMETIC_V<std::remove_cv_t<std::remove_reference_t<ArgsT>>> && ...)>>
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

  template <typename U = value_type, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<U>>>
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
            typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<U> && multiprecision::IS_COMPLEX_V<T>>>
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
    // Blocking multiplication might be implemented in the future, but it only makes sense for large matrices.
    Matrix<value_type, ROWS, OTHER_COLS> result{math::ZERO};
    for (std::uint16_t row = 0U; row < ROWS; ++row)
    {
      for (std::uint16_t col = 0U; col < COLS; ++col)
      {
        const auto lhs_val = lhs(row, col);
        for (std::uint16_t other_col = 0U; other_col < OTHER_COLS; ++other_col)
        {
          result(row, other_col) += lhs_val * rhs(col, other_col);
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

template <typename T>
using Matrix2x2 = Matrix<T, 2, 2>;
using Matrix2x2F = Matrix2x2<float>;
using Matrix2x2D = Matrix2x2<double>;
using Matrix2x2Q16 = Matrix2x2<multiprecision::FixedPoint16>;
using Matrix2x2Q32 = Matrix2x2<multiprecision::FixedPoint32>;

template <typename T>
using Matrix3x3 = Matrix<T, 3, 3>;
using Matrix3x3F = Matrix3x3<float>;
using Matrix3x3D = Matrix3x3<double>;
using Matrix3x3Q16 = Matrix3x3<multiprecision::FixedPoint16>;
using Matrix3x3Q32 = Matrix3x3<multiprecision::FixedPoint32>;

template <typename T>
using Matrix4x4 = Matrix<T, 4, 4>;
using Matrix4x4F = Matrix4x4<float>;
using Matrix4x4D = Matrix4x4<double>;
using Matrix4x4Q16 = Matrix4x4<multiprecision::FixedPoint16>;
using Matrix4x4Q32 = Matrix4x4<multiprecision::FixedPoint32>;

} // namespace rtw::math
