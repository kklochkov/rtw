#pragma once

#include "math/interpolation.h"
#include "math/vector.h"

namespace rtw::sw_renderer {

template <typename T, std::uint16_t N = 2U>
class TexCoord : math::Matrix<T, N, 1>
{
public:
  using Matrix = math::Matrix<T, N, 1>;
  using Vector = math::Vector<T, N>;

  using Matrix::Matrix;
  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;

  constexpr explicit TexCoord(const Matrix& matrix) : Matrix(matrix) {}
  constexpr explicit TexCoord(const Vector& vector) : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const { return Vector{as_matrix()}; }
  constexpr explicit operator Matrix() const { return as_matrix(); }

  constexpr value_type u() const noexcept { return operator[](0); }
  constexpr reference u() noexcept { return operator[](0); }

  constexpr value_type s() const noexcept { return u(); }
  constexpr reference s() noexcept { return u(); }

  constexpr value_type v() const noexcept { return operator[](1); }
  constexpr reference v() noexcept { return operator[](1); }

  constexpr value_type t() const noexcept { return v(); }
  constexpr reference t() noexcept { return v(); }

  constexpr TexCoord& operator*=(const value_type rhs)
  {
    as_matrix() *= rhs;
    return *this;
  }

  constexpr TexCoord& operator/=(const value_type rhs)
  {
    as_matrix() /= rhs;
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline TexCoord operator+(const TexCoord& lhs, const TexCoord& rhs)
  {
    return TexCoord{lhs.as_matrix() + rhs.as_matrix()};
  }

  friend constexpr inline TexCoord operator*(const TexCoord& lhs, const value_type rhs)
  {
    return TexCoord{lhs.as_matrix() * rhs};
  }

  friend constexpr inline TexCoord operator*(const value_type lhs, const TexCoord& rhs) { return rhs * lhs; }

  friend constexpr inline bool operator==(const TexCoord& lhs, const TexCoord& rhs)
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr inline TexCoord operator/(const TexCoord& lhs, const value_type rhs)
  {
    return TexCoord{lhs.as_matrix() / rhs};
  }

  friend constexpr inline bool operator!=(const TexCoord& lhs, const TexCoord& rhs) { return !(lhs == rhs); }

  friend inline std::ostream& operator<<(std::ostream& os, const TexCoord& coord)
  {
    os << "TexCoord" << N;
    return coord.as_matrix().operator<<(os);
  }
  /// @}
};

template <typename T>
using TexCoord2 = TexCoord<T, 2U>;
using TexCoord2f = TexCoord<float>;
using TexCoord2d = TexCoord<double>;

template <typename T>
constexpr inline TexCoord2<T> lerp(const TexCoord2<T>& lhs, const TexCoord2<T>& rhs, const T t)
{
  return TexCoord2<T>{math::lerp(lhs.as_matrix(), rhs.as_matrix(), t)};
}

} // namespace rtw::sw_renderer
