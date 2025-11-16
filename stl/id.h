#pragma once

#include <cstdint>

namespace rtw::stl
{

struct Id
{
  using ID_TYPE = std::uint32_t;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator ID_TYPE() const noexcept { return id; }
  constexpr bool operator==(const Id& other) const noexcept { return id == other.id; }
  constexpr bool operator!=(const Id& other) const noexcept { return !(*this == other); }

  constexpr Id& operator++() noexcept
  {
    ++id;
    return *this;
  }

  constexpr Id operator++(int) noexcept
  {
    const auto result = *this;
    ++(*this);
    return result;
  }

  constexpr Id& operator--() noexcept
  {
    --id;
    return *this;
  }

  constexpr Id operator--(int) noexcept
  {
    const auto result = *this;
    --(*this);
    return result;
  }

  ID_TYPE id{};
};

} // namespace rtw::stl
