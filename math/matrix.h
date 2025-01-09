#pragma once

#include <array>
#include <cassert>
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
/// @tparam N The number of rows.
/// @tparam M The number of columns.
template <typename T, std::uint16_t N, std::uint16_t M>
class Matrix
{
  constexpr static std::uint32_t SIZE = N * M;

  template <std::uint16_t ROWS>
  constexpr static bool HAS_X = (ROWS > 0);

  template <std::uint16_t ROWS>
  constexpr static bool HAS_Y = (ROWS > 1);

  template <std::uint16_t ROWS>
  constexpr static bool HAS_Z = (ROWS > 2);

  template <std::uint16_t ROWS>
  constexpr static std::uint16_t W_INDEX = (ROWS > 3 ? 3 : 2);

  template <typename U = T, std::uint16_t P, std::uint16_t Q, std::uint16_t... INDEX>
  constexpr static std::array<T, SIZE> from_matrix(const Matrix<U, P, Q>& matrix,
                                                   std::integer_sequence<std::uint16_t, INDEX...> /*index*/) noexcept
  {
    static_assert((P * Q) <= SIZE, "Too many elements");
    return std::array<T, SIZE>{matrix[INDEX]...};
  }

  template <std::uint16_t... INDEX>
  constexpr static std::array<T, SIZE> make_identity(std::integer_sequence<std::uint16_t, INDEX...> /*index*/) noexcept
  {
    return std::array<T, SIZE>{(INDEX % (N + 1) == 0 ? T{1} : T{0})...};
  }

public:
  using value_type = T;
  using iterator = typename std::array<value_type, SIZE>::iterator;
  using const_iterator = typename std::array<value_type, SIZE>::const_iterator;
  using pointer = typename std::array<value_type, SIZE>::pointer;
  using const_pointer = typename std::array<value_type, SIZE>::const_pointer;
  using reference = typename std::array<value_type, SIZE>::reference;

  static_assert(N > 0, "N (number of rows) must be greater than 0");
  static_assert(M > 0, "M (number of cols) must be greater than 0");
  static_assert(std::is_arithmetic_v<value_type>, "T must be arithmetic");

  constexpr explicit Matrix(UninitializedTag /*tag*/) noexcept {}
  constexpr explicit Matrix(ZeroTag /*tag*/) noexcept : data_{value_type{0}} {}
  constexpr explicit Matrix(IdentityTag /*tag*/) noexcept
      : data_{make_identity(std::make_integer_sequence<std::uint16_t, SIZE>{})}
  {
  }
  constexpr Matrix() noexcept : Matrix{math::ZERO} {}

  template <typename... ArgsT,
            typename = std::enable_if_t<(sizeof...(ArgsT) <= SIZE)
                                        && (std::is_arithmetic_v<std::remove_reference_t<ArgsT>> && ...)>>
  constexpr explicit Matrix(ArgsT&&... args) noexcept : data_{std::forward<ArgsT>(args)...}
  {
  }

  template <typename U = value_type, std::uint16_t P, std::uint16_t Q,
            typename = std::enable_if_t<std::is_same_v<U, value_type> && (P <= N) && (Q <= M)>>
  constexpr explicit Matrix(const Matrix<U, P, Q>& matrix) noexcept
      : data_{from_matrix(matrix, std::make_integer_sequence<std::uint16_t, P * Q>{})}
  {
  }

  constexpr std::uint16_t rows() const noexcept { return N; }
  constexpr std::uint16_t cols() const noexcept { return M; }
  constexpr std::uint32_t size() const noexcept { return SIZE; }

  template <typename U = value_type, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
  constexpr Matrix<U, N, M> cast() const noexcept
  {
    Matrix<U, N, M> result{math::UNINITIALIZED};
    for (std::uint32_t i = 0U; i < SIZE; ++i)
    {
      result[i] = static_cast<U>(data_[i]);
    }
    return result;
  }

  constexpr reference operator()(const std::uint16_t row, const std::uint16_t col) noexcept
  {
    assert(row < N);
    assert(col < M);
    return data_[row * M + col];
  }
  constexpr value_type operator()(const std::uint16_t row, const std::uint16_t col) const noexcept
  {
    assert(row < N);
    assert(col < M);
    return data_[row * M + col];
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
  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_X<ROWS>>>
  constexpr value_type x() const noexcept
  {
    return operator[](0);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_X<ROWS>>>
  constexpr reference x() noexcept
  {
    return operator[](0);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Y<ROWS>>>
  constexpr value_type y() const noexcept
  {
    return operator[](1);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Y<ROWS>>>
  constexpr reference y() noexcept
  {
    return operator[](1);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Z<ROWS>>>
  constexpr value_type z() const noexcept
  {
    return operator[](2);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Z<ROWS>>>
  constexpr reference z() noexcept
  {
    return operator[](2);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Z<ROWS>>>
  constexpr value_type w() const noexcept
  {
    return operator[](W_INDEX<ROWS>);
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<HAS_Z<ROWS>>>
  constexpr reference w() noexcept
  {
    return operator[](W_INDEX<ROWS>);
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
  template <std::uint16_t ROWS = N, typename = std::enable_if_t<(ROWS > 2)>>
  constexpr Matrix<value_type, 2, 1> xy() const noexcept
  {
    return Matrix<value_type, 2, 1>{x(), y()};
  }

  template <std::uint16_t ROWS = N, typename = std::enable_if_t<(ROWS > 3)>>
  constexpr Matrix<value_type, 3, 1> xyz() const noexcept
  {
    return Matrix<value_type, 3, 1>{x(), y(), z()};
  }
  /// @}

  constexpr Matrix<T, M, 1> column(const std::uint16_t col) const noexcept
  {
    assert(col < M);
    Matrix<T, M, 1> result{math::UNINITIALIZED};
    for (std::uint16_t i = 0U; i < M; ++i)
    {
      result[i] = (*this)(i, col);
    }
    return result;
  }

  constexpr Matrix<T, N, 1> row(const std::uint16_t row) const noexcept
  {
    assert(row < N);
    Matrix<T, N, 1> result{math::UNINITIALIZED};
    for (std::uint16_t i = 0U; i < N; ++i)
    {
      result[i] = (*this)(row, i);
    }
    return result;
  }

  template <std::uint16_t ROWS = N, std::uint16_t COLUMNS = M,
            typename = std::enable_if_t<(ROWS == COLUMNS) && (ROWS > 2) && (COLUMNS > 2)>>
  constexpr Matrix<T, ROWS - 1, COLUMNS - 1> minor(const std::uint16_t row, const std::uint16_t col) const noexcept
  {
    assert(row < ROWS);
    assert(col < COLUMNS);
    Matrix<T, ROWS - 1, COLUMNS - 1> result{math::UNINITIALIZED};
    for (std::uint16_t i = 0U; i < ROWS - 1; ++i)
    {
      const std::uint16_t r = i < row ? i : i + 1;
      for (std::uint16_t j = 0U; j < COLUMNS - 1; ++j)
      {
        const std::uint16_t c = j < col ? j : j + 1;
        result(i, j) = (*this)(r, c);
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
    if constexpr (M == 1)
    {
      for (std::uint16_t i = 0U; i < N; ++i)
      {
        os << (*this)(i, 0);
        if (i < N - 1U)
        {
          os << ' ';
        }
      }
    }
    else
    {
      os << '\n';
      for (std::uint16_t i = 0U; i < N; ++i)
      {
        for (std::uint16_t j = 0U; j < M; ++j)
        {
          os << std::setw(10) << (*this)(i, j);
          if (j < M - 1U)
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

  template <std::uint16_t P>
  friend constexpr Matrix<value_type, N, P> operator*(const Matrix& lhs, const Matrix<value_type, M, P>& rhs) noexcept
  {
    // TODO: make this more cache friendly.
    Matrix<value_type, N, P> result{math::ZERO};
    for (std::uint16_t i = 0U; i < N; ++i)
    {
      for (std::uint16_t j = 0U; j < P; ++j)
      {
        for (std::uint16_t k = 0U; k < M; ++k)
        {
          result(i, j) += lhs(i, k) * rhs(k, j);
        }
      }
    }
    return result;
  }

  template <std::uint16_t P>
  friend constexpr Matrix<value_type, N, P> operator*(const Matrix<value_type, N, M>& lhs, const Matrix& rhs) noexcept
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
    os << "Matrix" << N << "x" << M;
    return matrix.operator<<(os);
  }
  /// @}

private:
  std::array<value_type, SIZE> data_;
};

/// Compute the transpose of a matrix.
/// @param[in] matrix The matrix.
/// @return The transpose of the matrix.
template <typename T, std::uint16_t N, std::uint16_t M>
constexpr Matrix<T, M, N> transpose(const Matrix<T, N, M>& matrix) noexcept
{
  Matrix<T, M, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < N; ++i)
  {
    for (std::uint16_t j = 0U; j < M; ++j)
    {
      result(j, i) = matrix(i, j);
    }
  }
  return result;
}

template <typename T>
using Matrix2x2 = Matrix<T, 2, 2>;
using Matrix2x2f = Matrix2x2<float>;
using Matrix2x2d = Matrix2x2<double>;

template <typename T>
using Matrix3x3 = Matrix<T, 3, 3>;
using Matrix3x3f = Matrix3x3<float>;
using Matrix3x3d = Matrix3x3<double>;

template <typename T>
using Matrix4x4 = Matrix<T, 4, 4>;
using Matrix4x4f = Matrix4x4<float>;
using Matrix4x4d = Matrix4x4<double>;

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
  return inv_det * Matrix<T, 2, 2>{d, -b, -c, a};
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
  return inv_det * Matrix<T, 3, 3>{res_a, res_d, res_g, res_b, res_e, res_h, res_c, res_f, res_i};
}

template <typename T, std::uint16_t N>
constexpr T dot(const Matrix<T, N, 1>& lhs, const Matrix<T, N, 1>& rhs) noexcept
{
  T result{0};
  for (std::uint16_t i = 0U; i < N; ++i)
  {
    result += lhs[i] * rhs[i];
  }
  return result;
}

template <typename T, std::uint16_t N>
constexpr T norm2(const Matrix<T, N, 1>& vector) noexcept
{
  return dot(vector, vector);
}

template <typename T, std::uint16_t N>
constexpr T norm(const Matrix<T, N, 1>& vector) noexcept
{
  return std::sqrt(norm2(vector));
}

template <typename T, std::uint16_t N>
constexpr Matrix<T, N, 1> normalize(const Matrix<T, N, 1>& vector) noexcept
{
  const auto n = norm(vector);
  assert(n != T{0});
  return vector / n;
}

} // namespace rtw::math
