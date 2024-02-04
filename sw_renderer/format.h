#pragma once

#include "sw_renderer/barycentric.h"
#include "sw_renderer/tex_coord.h"

#include <fmt/ostream.h>

template <typename T>
struct fmt::formatter<rtw::sw_renderer::Barycentric3<T>> : fmt::ostream_formatter
{
};

template <typename T>
struct fmt::formatter<rtw::sw_renderer::TexCoord2<T>> : fmt::ostream_formatter
{
};
