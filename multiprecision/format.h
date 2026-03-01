#pragma once

#include "multiprecision/ostream.h" // IWYU pragma: keep

#include <fmt/ostream.h>

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::multiprecision::Int<T>> : ostream_formatter
{};

template <typename T, std::uint8_t FRAC_BITS, typename SaturationT>
struct formatter<rtw::multiprecision::FixedPoint<T, FRAC_BITS, SaturationT>> : ostream_formatter
{};

template <typename T>
struct formatter<rtw::multiprecision::Rational<T>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
