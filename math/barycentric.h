#pragma once

#include "math/matrix.h"
#include "math/vector.h"

namespace rtw::math
{

template <typename T>
class Barycentric;

template <typename T>
std::ostream& operator<<(std::ostream& os, const Barycentric<T>& coord) noexcept;

template <typename T>
class Barycentric : Matrix<T, 3, 1>
{
public:
  using Matrix = Matrix<T, 3, 1>;
  using Vector = Vector<T, 3>;

  using typename Matrix::reference;
  using typename Matrix::value_type;
  using Matrix::operator[];
  using Matrix::begin;
  using Matrix::cbegin;
  using Matrix::cend;
  using Matrix::data;
  using Matrix::end;

  constexpr Barycentric() noexcept = default;

  constexpr explicit Barycentric(UninitializedTag tag) noexcept : Matrix(tag) {}
  constexpr Barycentric(InitializeWithValueTag tag, const T value) noexcept : Matrix(tag, value) {}

  constexpr Barycentric(const T w0, const T w1, const T w2) noexcept : Matrix(w0, w1, w2) {}

  constexpr explicit Barycentric(const Matrix& matrix) noexcept : Matrix(matrix) {}
  constexpr explicit Barycentric(const Vector& vector) noexcept : Matrix(vector.as_matrix()) {}

  constexpr Matrix& as_matrix() noexcept { return static_cast<Matrix&>(*this); }
  constexpr const Matrix& as_matrix() const noexcept { return static_cast<const Matrix&>(*this); }

  constexpr explicit operator Vector() const noexcept { return Vector{as_matrix()}; }
  constexpr explicit operator Matrix() const noexcept { return as_matrix(); }

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
  friend constexpr bool operator==(const Barycentric& lhs, const Barycentric& rhs) noexcept
  {
    return lhs.as_matrix() == rhs.as_matrix();
  }

  friend constexpr bool operator!=(const Barycentric& lhs, const Barycentric& rhs) noexcept { return !(lhs == rhs); }
  /// @}

  friend std::ostream& operator<< <T>(std::ostream& os, const Barycentric& coord) noexcept;
};

using BarycentricF = Barycentric<float>;
using BarycentricD = Barycentric<double>;
using BarycentricQ16 = Barycentric<multiprecision::FixedPoint16>;
using BarycentricQ32 = Barycentric<multiprecision::FixedPoint32>;

} // namespace rtw::math
