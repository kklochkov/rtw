#pragma once

#include "fixed_point/format.h" // IWYU pragma: export
#include "math/barycentric.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"

#include <fmt/ostream.h>

#include <iomanip>
#include <ostream>

namespace rtw::math
{

namespace details
{

template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
std::ostream& operator<<(std::ostream& os, const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto temp_format = os.flags();

  os << std::fixed << std::setprecision(4) << '[';
  if constexpr (COLS == 1)
  {
    for (std::uint16_t row = 0U; row < ROWS; ++row)
    {
      os << matrix(row, 0);
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
        os << std::setw(10) << matrix(row, col);
        if (col < COLS - 1U)
        {
          os << ' ';
        }
      }
      os << '\n';
    }
  }
  os << ']';

  os.flags(temp_format);
  return os;
}

} // namespace details

template <typename T, std::uint16_t ROWS, std::uint16_t COLS>
std::ostream& operator<<(std::ostream& os, const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  os << "Matrix" << ROWS << "x" << COLS;
  return details::operator<<(os, matrix);
}

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vector) noexcept
{
  os << "Vector" << N;
  return details::operator<<(os, vector);
}

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Point<T, N>& point) noexcept
{
  os << "Point" << N;
  return details::operator<<(os, point);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Barycentric<T>& coord) noexcept
{
  os << "Barycentric";
  return math::details::operator<<(os, coord);
}

} // namespace rtw::math

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T, std::uint16_t N, std::uint16_t M>
struct formatter<rtw::math::Matrix<T, N, M>> : ostream_formatter
{};

template <typename T, std::uint16_t N>
struct formatter<rtw::math::Vector<T, N>> : ostream_formatter
{};

template <typename T, std::uint16_t N>
struct formatter<rtw::math::Point<T, N>> : ostream_formatter
{};

template <typename T>
struct formatter<rtw::math::Barycentric<T>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
