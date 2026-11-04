#pragma once

#include <cstdint>

namespace rtw::stl
{

/// Non-owning source-location information.
/// Explicitly supplied file and function names must be non-null, null-terminated C strings
/// whose storage remains valid while the location is used. Names are not owned or copied.
/// The default builtin names have static lifetime and satisfy these requirements.
class SourceLocation
{
public:
  constexpr SourceLocation() noexcept = default;

  constexpr static SourceLocation current(const char* file_name = __builtin_FILE(),
                                          const char* function_name = __builtin_FUNCTION(),
                                          const std::uint32_t line = __builtin_LINE()) noexcept
  {
    return SourceLocation{file_name, function_name, line};
  }

  constexpr const char* file_name() const noexcept { return file_name_; }
  constexpr const char* function_name() const noexcept { return function_name_; }
  constexpr std::uint32_t line() const noexcept { return line_; }

private:
  constexpr SourceLocation(const char* file_name, const char* function_name, const std::uint32_t line) noexcept
      : file_name_{file_name}, function_name_{function_name}, line_{line}
  {
  }

  const char* file_name_{""};
  const char* function_name_{""};
  std::uint32_t line_{};
};

} // namespace rtw::stl
