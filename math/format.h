#pragma once

#include "math/barycentric.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"
#include "multiprecision/format.h"

#include <fmt/ostream.h>

#include <iomanip>
#include <ostream>

namespace rtw::math
{

namespace details
{

enum class FormatType : std::uint8_t
{
  MATRIX,
  VECTOR,
};

template <FormatType FORMAT, typename T, std::uint16_t ROWS, std::uint16_t COLS>
std::ostream& operator<<(std::ostream& os, const Matrix<T, ROWS, COLS>& matrix) noexcept
{
  const auto temp_format = os.flags();

  os << std::fixed << std::setprecision(6) << '[';
  if constexpr (FORMAT == FormatType::VECTOR)
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
  else if constexpr (FORMAT == FormatType::MATRIX)
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
  else
  {
    static_assert(FORMAT == FormatType::VECTOR || FORMAT == FormatType::MATRIX, "Invalid format type");
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
  return details::operator<< <details::FormatType::MATRIX>(os, matrix);
}

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vector) noexcept
{
  os << "Vector" << N;
  return details::operator<< <details::FormatType::VECTOR>(os, vector);
}

template <typename T, std::uint16_t N>
std::ostream& operator<<(std::ostream& os, const Point<T, N>& point) noexcept
{
  os << "Point" << N;
  return details::operator<< <details::FormatType::VECTOR>(os, point);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Barycentric<T>& coord) noexcept
{
  os << "Barycentric";
  return math::details::operator<< <details::FormatType::VECTOR>(os, coord);
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
