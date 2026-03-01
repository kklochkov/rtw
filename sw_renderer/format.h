#pragma once

#include "sw_renderer/ostream.h" // IWYU pragma: keep

#include <fmt/ostream.h>

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::sw_renderer::TexCoord<T>> : ostream_formatter
{};

template <>
struct formatter<rtw::sw_renderer::Color> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
