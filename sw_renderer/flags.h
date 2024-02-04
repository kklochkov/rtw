#pragma once

#include <cstdint>
#include <type_traits>

namespace rtw::sw_renderer {

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr inline T operator|(const T lhs, const T rhs)
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr inline T operator&(const T lhs, const T rhs)
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr inline T operator^(const T lhs, const T rhs)
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr inline T operator~(const T lhs)
{
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(lhs));
}

template <typename T>
class Flags
{
public:
  static_assert(std::is_enum_v<T>, "T must be an enum type.");

  using value_type = T;

  constexpr Flags() : flags_{static_cast<value_type>(0)} {}
  constexpr Flags(const value_type flags) : flags_{flags} {}

  constexpr operator value_type() const { return flags_; }
  constexpr operator bool() const { return flags_ != static_cast<value_type>(0); }

  constexpr void set(const value_type flags, const bool enabled = true)
  {
    flags_ = enabled ? flags_ | flags : flags_ & ~flags;
  }
  constexpr bool test(const value_type flags) const { return (flags_ & flags) == flags; }

  constexpr Flags operator|(const Flags& rhs) const { return Flags{flags_ | rhs.flags_}; }
  constexpr Flags operator&(const Flags& rhs) const { return Flags{flags_ & rhs.flags_}; }
  constexpr Flags operator^(const Flags& rhs) const { return Flags{flags_ ^ rhs.flags_}; }
  constexpr Flags operator~() const { return Flags{~flags_}; }

  constexpr Flags& operator|=(const Flags& rhs)
  {
    flags_ = flags_ | rhs.flags_;
    return *this;
  }
  constexpr Flags& operator|=(const value_type rhs) { return operator|=(Flags{rhs}); }

  constexpr Flags& operator&=(const Flags& rhs)
  {
    flags_ = flags_ & rhs.flags_;
    return *this;
  }
  constexpr Flags& operator&=(const value_type rhs) { return operator&=(Flags{rhs}); }

  constexpr Flags& operator^=(const Flags& rhs)
  {
    flags_ = flags_ ^ rhs.flags_;
    return *this;
  }
  constexpr Flags& operator^=(const value_type rhs) { return operator^=(Flags{rhs}); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr inline Flags operator|(const value_type lhs, const Flags& rhs) { return Flags{lhs} | rhs; }
  friend constexpr inline Flags operator&(const value_type lhs, const Flags& rhs) { return Flags{lhs} & rhs; }
  friend constexpr inline Flags operator^(const value_type lhs, const Flags& rhs) { return Flags{lhs} ^ rhs; }

  friend constexpr inline bool operator==(const value_type lhs, const Flags& rhs) { return lhs == rhs.flags_; }
  friend constexpr inline bool operator!=(const value_type lhs, const Flags& rhs) { return lhs != rhs.flags_; }
  /// @}
private:
  value_type flags_;
};

} // namespace rtw::sw_renderer
