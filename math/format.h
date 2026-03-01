#pragma once

#include "math/ostream.h" // IWYU pragma: keep

#include <fmt/ostream.h>

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T, std::uint16_t N, std::uint16_t M, rtw::math::MemoryOrder MEMORY_ORDER>
struct formatter<rtw::math::Matrix<T, N, M, MEMORY_ORDER>> : ostream_formatter
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
