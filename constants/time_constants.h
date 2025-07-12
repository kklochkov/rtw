#pragma once

#include <chrono>

namespace rtw::time_constants
{

using Seconds = std::chrono::duration<float, std::centi>;
using Milliseconds = std::chrono::duration<float, std::milli>;

} // namespace rtw::time_constants
