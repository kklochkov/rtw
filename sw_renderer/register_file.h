#pragma once

#include "math/interpolation.h"
#include "math/vector.h"

#include <array>
#include <cstdint>

namespace rtw::sw_renderer
{

/// Low-level varyings substrate: a fixed array of `CAPACITY` vec4 slots with element-wise arithmetic.
///
/// This is the representation the pipeline core (clipper, rasterizer, perspective-correct interpolation) operates on.
///
/// `CAPACITY` is the exact number of active slots; arithmetic and interpolation process all of them.
/// Storage is typically `single_precision`, but the type is generic so perspective-correct interpolation may be carried
/// out in a wider type when required.
///
/// @tparam T The scalar component type (e.g. float, FixedPoint16).
/// @tparam CAPACITY The number of vec4 slots.
template <typename T, std::uint16_t CAPACITY>
class RegisterFile
{
  using StorageType = std::array<math::Vector4<T>, CAPACITY>;

public:
  using value_type = typename StorageType::value_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;

  constexpr std::uint16_t size() const noexcept { return CAPACITY; }

  constexpr reference operator[](const std::uint16_t index) noexcept { return storage_[index]; }
  constexpr const_reference operator[](const std::uint16_t index) const noexcept { return storage_[index]; }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

  constexpr pointer data() noexcept { return storage_.data(); }
  constexpr const_pointer data() const noexcept { return storage_.data(); }

  constexpr RegisterFile& operator+=(const RegisterFile& rhs) noexcept
  {
    for (std::uint16_t i = 0U; i < size(); ++i)
    {
      storage_[i] += rhs.storage_[i];
    }
    return *this;
  }

  constexpr RegisterFile& operator*=(const T rhs) noexcept
  {
    for (std::uint16_t i = 0U; i < size(); ++i)
    {
      storage_[i] *= rhs;
    }
    return *this;
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr RegisterFile operator+(RegisterFile lhs, const RegisterFile& rhs) noexcept { return lhs += rhs; }
  friend constexpr RegisterFile operator*(RegisterFile lhs, const T rhs) noexcept { return lhs *= rhs; }
  friend constexpr RegisterFile operator*(const T lhs, RegisterFile rhs) noexcept { return rhs *= lhs; }
  friend constexpr bool operator==(const RegisterFile& lhs, const RegisterFile& rhs) noexcept
  {
    for (std::uint16_t i = 0U; i < lhs.size(); ++i)
    {
      if (lhs.storage_[i] != rhs.storage_[i])
      {
        return false;
      }
    }
    return true;
  }
  friend constexpr bool operator!=(const RegisterFile& lhs, const RegisterFile& rhs) noexcept { return !(lhs == rhs); }
  /// @}

private:
  StorageType storage_{};
};

template <typename T, std::uint16_t CAPACITY>
constexpr RegisterFile<T, CAPACITY> lerp(const RegisterFile<T, CAPACITY>& lhs, const RegisterFile<T, CAPACITY>& rhs,
                                         const T t) noexcept
{
  RegisterFile<T, CAPACITY> result{};
  for (std::uint16_t i = 0U; i < lhs.size(); ++i)
  {
    result[i] = math::lerp(lhs[i], rhs[i], t);
  }
  return result;
}

} // namespace rtw::sw_renderer
