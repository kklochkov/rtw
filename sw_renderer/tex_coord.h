#pragma once

#include "math/interpolation.h"
#include "math/vector.h"

namespace rtw::sw_renderer
{

template <typename T, std::uint16_t N>
class TexCoord;

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const TexCoord<T, N>& coord) noexcept;

template <typename T, std::uint16_t N = 2U>
class TexCoord : math::Matrix<T, N, 1>
{
public:
  using Matrix = math::Matrix<T, N, 1>;
  using Vector = math::Vector<T, N>;

  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;

  constexpr TexCoord() noexcept = default;

  constexpr explicit TexCoord(math::UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr TexCoord(math::InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}

  constexpr TexCoord(const T u, const T v) noexcept : Matrix(u, v) {}

  constexpr explicit TexCoord(const Matrix& matrix) noexcept : Matrix(matrix) {}
  constexpr explicit TexCoord(const Vector& vector) noexcept : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const noexcept { return Vector{as_matrix()}; }
  constexpr explicit operator Matrix() const noexcept { return as_matrix(); }

  constexpr value_type u() const noexcept { return operator[](0); }
  constexpr reference u() noexcept { return operator[](0); }

  constexpr value_type s() const noexcept { return u(); }
  constexpr reference s() noexcept { return u(); }

  constexpr value_type v() const noexcept { return operator[](1); }
  constexpr reference v() noexcept { return operator[](1); }

  constexpr value_type t() const noexcept { return v(); }
  constexpr reference t() noexcept { return v(); }

  constexpr TexCoord& operator*=(const value_type rhs) noexcept
  {
    as_matrix() *= rhs;
    return *this;
  }

  constexpr TexCoord& operator/=(const value_type rhs) noexcept
  {
    as_matrix() /= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr TexCoord operator+(const TexCoord& lhs, const TexCoord& rhs) noexcept
  {
    return TexCoord{lhs.as_matrix() + rhs.as_matrix()};
  }

  friend constexpr TexCoord operator*(const TexCoord& lhs, const value_type rhs) noexcept
  {
    return TexCoord{lhs.as_matrix() * rhs};
  }

  friend constexpr TexCoord operator*(const value_type lhs, const TexCoord& rhs) noexcept { return rhs * lhs; }

  friend constexpr bool operator==(const TexCoord& lhs, const TexCoord& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr TexCoord operator/(const TexCoord& lhs, const value_type rhs) noexcept
  {
    return TexCoord{lhs.as_matrix() / rhs};
  }

  friend constexpr bool operator!=(const TexCoord& lhs, const TexCoord& rhs) noexcept { return !(lhs == rhs); }
  /// @}

  friend std::ostream& operator<< <T, N>(std::ostream& os, const TexCoord& coord) noexcept;
};

template <typename T>
using TexCoord2 = TexCoord<T, 2U>;
using TexCoord2F = TexCoord<float>;
using TexCoord2D = TexCoord<double>;

template <typename T>
constexpr TexCoord2<T> lerp(const TexCoord2<T>& lhs, const TexCoord2<T>& rhs, const T t) noexcept
{
  return TexCoord2<T>{math::lerp(lhs.as_matrix(), rhs.as_matrix(), t)};
}

} // namespace rtw::sw_renderer
