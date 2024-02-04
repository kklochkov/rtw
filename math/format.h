#pragma once

#include "math/point.h"

#include <fmt/ostream.h>

template <typename T, std::uint16_t N, std::uint16_t M>
struct fmt::formatter<rtw::math::Matrix<T, N, M>> : fmt::ostream_formatter
{
};

template <typename T, std::uint16_t N>
struct fmt::formatter<rtw::math::Vector<T, N>> : fmt::ostream_formatter
{
};

template <typename T, std::uint16_t N>
struct fmt::formatter<rtw::math::Point<T, N>> : fmt::ostream_formatter
{
};
