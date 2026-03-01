#pragma once

#include "stl/string_view.h"

namespace rtw::stl
{

class SourceLocation
{
public:
  constexpr SourceLocation() noexcept = default;

  constexpr static SourceLocation current(const StringView file_name = __builtin_FILE(),
                                          const StringView function_name = __builtin_FUNCTION(),
                                          const std::uint32_t line = __builtin_LINE()) noexcept
  {
    return SourceLocation{file_name, function_name, line};
  }

  constexpr StringView file_name() const noexcept { return file_name_; }
  constexpr StringView function_name() const noexcept { return function_name_; }
  constexpr std::uint32_t line() const noexcept { return line_; }

private:
  constexpr SourceLocation(const StringView file_name, const StringView function_name,
                           const std::uint32_t line) noexcept
      : file_name_{file_name}, function_name_{function_name}, line_{line}
  {
  }

  StringView file_name_;
  StringView function_name_;
  std::uint32_t line_{};
};

} // namespace rtw::stl
