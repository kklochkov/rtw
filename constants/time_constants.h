#pragma once

#include <chrono>

namespace rtw::time_constants
{

/// Duration type representing seconds as a single-precision float.
using Seconds = std::chrono::duration<float>;

/// Duration type representing milliseconds as a single-precision float.
using Milliseconds = std::chrono::duration<float, std::milli>;

} // namespace rtw::time_constants
