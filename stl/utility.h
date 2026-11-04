#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace rtw::stl
{

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-pro-bounds-pointer-arithmetic)
template <typename ContainerT, typename T = std::remove_reference_t<decltype(std::declval<ContainerT&>()[0])>>
constexpr T& at_unsafe(ContainerT& container, const std::size_t index) noexcept
{
  return container[index];
}

template <typename ContainerT, typename T = std::remove_reference_t<decltype(std::declval<const ContainerT&>()[0])>>
constexpr const T& at_unsafe(const ContainerT& container, const std::size_t index) noexcept
{
  return container[index];
}

template <typename T, std::size_t N>
constexpr T& at_unsafe(T (&container)[N], const std::size_t index) noexcept
{
  return container[index];
}

template <typename T, std::size_t N>
constexpr const T& at_unsafe(const T (&container)[N], const std::size_t index) noexcept
{
  return container[index];
}
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-pro-bounds-pointer-arithmetic)

} // namespace rtw::stl
