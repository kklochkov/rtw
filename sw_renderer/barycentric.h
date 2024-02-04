#pragma once

#include "math/vector.h"

namespace rtw::sw_renderer {

template <typename T, std::uint16_t N = 3U>
class Barycentric : math::Matrix<T, N, 1>
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

  constexpr explicit Barycentric(const Matrix& matrix) : Matrix(matrix) {}
  constexpr explicit Barycentric(const Vector& vector) : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const { return Vector{as_matrix()}; }
  constexpr explicit operator Matrix() const { return as_matrix(); }

  constexpr value_type w0() const noexcept { return operator[](0); }
  constexpr reference w0() noexcept { return operator[](0); }

  constexpr value_type lambda0() const noexcept { return w0(); }
  constexpr reference lambda0() noexcept { return w0(); }

  constexpr value_type alpha() const noexcept { return w0(); }
  constexpr reference alpha() noexcept { return w0(); }

  constexpr value_type w1() const noexcept { return operator[](1); }
  constexpr reference w1() noexcept { return operator[](1); }

  constexpr value_type lambda1() const noexcept { return w1(); }
  constexpr reference lambda1() noexcept { return w1(); }

  constexpr value_type beta() const noexcept { return w1(); }
  constexpr reference beta() noexcept { return w1(); }

  constexpr value_type w2() const noexcept { return operator[](2); }
  constexpr reference w2() noexcept { return operator[](2); }

  constexpr value_type lambda2() const noexcept { return w2(); }
  constexpr reference lambda2() noexcept { return w2(); }

  constexpr value_type gamma() const noexcept { return w2(); }
  constexpr reference gamma() noexcept { return w2(); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline bool operator==(const Barycentric& lhs, const Barycentric& rhs)
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr inline bool operator!=(const Barycentric& lhs, const Barycentric& rhs) { return !(lhs == rhs); }

  friend inline std::ostream& operator<<(std::ostream& os, const Barycentric& coord)
  {
    os << "Barycentric" << N;
    return coord.as_matrix().operator<<(os);
  }
  /// @}
};

template <typename T>
using Barycentric3 = Barycentric<T, 3U>;
using Barycentric3f = Barycentric3<float>;
using Barycentric3d = Barycentric3<double>;

} // namespace rtw::sw_renderer
