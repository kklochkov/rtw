#pragma once

#include "stl/ostream.h" // IWYU pragma: keep

#include <fmt/ostream.h>

// NOLINTBEGIN(readability-identifier-naming)
namespace fmt
{

template <typename T>
struct formatter<rtw::stl::Flags<T>> : ostream_formatter
{};

template <>
struct formatter<rtw::stl::StringView> : ostream_formatter
{};

template <typename DerivedT>
struct formatter<rtw::stl::GenericStaticString<DerivedT>> : ostream_formatter
{};

} // namespace fmt
// NOLINTEND(readability-identifier-naming)
