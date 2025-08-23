#pragma once

#include <type_traits>

namespace rtw::stl
{

namespace details
{

constexpr bool test_scoped_enum(...) noexcept { return true; }
constexpr bool test_scoped_enum(std::int32_t /*unused*/) noexcept { return false; }

template <typename T>
constexpr inline bool IS_SCOPED_ENUM_V = std::is_enum_v<T> && test_scoped_enum(T{});

} // namespace details

template <typename T>
class Flags
{
public:
  static_assert(std::is_enum_v<T>, "T must be an enum type.");

  using underlying_type = std::underlying_type_t<T>;
  using enum_type = T;

  constexpr Flags() noexcept : Flags{static_cast<underlying_type>(0)} {}
  constexpr explicit Flags(const T flag) noexcept : Flags{static_cast<underlying_type>(flag)} {}

  constexpr explicit operator underlying_type() const noexcept { return flags_; }
  constexpr explicit operator bool() const noexcept { return flags_ != static_cast<underlying_type>(0); }
  constexpr bool any() const noexcept { return static_cast<bool>(*this); }
  constexpr bool none() const noexcept { return !static_cast<bool>(*this); }

  constexpr void set(const T flag, const bool enabled = true) noexcept
  {
    if (enabled)
    {
      flags_ |= static_cast<underlying_type>(flag);
    }
    else
    {
      flags_ &= ~static_cast<underlying_type>(flag);
    }
  }

  constexpr void reset() noexcept { flags_ = static_cast<underlying_type>(0); }

  constexpr bool test(const T flag) const noexcept
  {
    return (flags_ & static_cast<underlying_type>(flag)) == static_cast<underlying_type>(flag);
  }

  constexpr Flags operator~() const noexcept { return Flags{static_cast<underlying_type>(~flags_)}; }

  constexpr Flags& operator|=(const Flags& rhs) noexcept
  {
    flags_ |= rhs.flags_;
    return *this;
  }
  constexpr Flags& operator|=(const T rhs) noexcept { return operator|=(Flags{rhs}); }

  constexpr Flags& operator&=(const Flags& rhs) noexcept
  {
    flags_ &= rhs.flags_;
    return *this;
  }
  constexpr Flags& operator&=(const T rhs) noexcept { return operator&=(Flags{rhs}); }

  constexpr Flags& operator^=(const Flags& rhs) noexcept
  {
    flags_ ^= rhs.flags_;
    return *this;
  }
  constexpr Flags& operator^=(const T rhs) noexcept { return operator^=(Flags{rhs}); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Flags operator|(Flags lhs, const Flags rhs) noexcept { return lhs |= rhs; }
  friend constexpr Flags operator|(Flags lhs, const T rhs) noexcept { return lhs |= rhs; }
  friend constexpr Flags operator|(const T lhs, Flags rhs) noexcept { return rhs |= lhs; }
  friend constexpr Flags operator&(Flags lhs, const Flags rhs) noexcept { return lhs &= rhs; }
  friend constexpr Flags operator&(Flags lhs, const T rhs) noexcept { return lhs &= rhs; }
  friend constexpr Flags operator&(const T lhs, Flags rhs) noexcept { return rhs &= lhs; }
  friend constexpr Flags operator^(Flags lhs, const Flags rhs) noexcept { return lhs ^= rhs; }
  friend constexpr Flags operator^(Flags lhs, const T rhs) noexcept { return lhs ^= rhs; }
  friend constexpr Flags operator^(const T lhs, Flags rhs) noexcept { return rhs ^= lhs; }

  friend constexpr bool operator==(const Flags& lhs, const Flags& rhs) noexcept { return lhs.flags_ == rhs.flags_; }
  friend constexpr bool operator==(const Flags& lhs, const T rhs) noexcept { return lhs.flags_ == Flags{rhs}.flags_; }
  friend constexpr bool operator==(const T lhs, const Flags& rhs) noexcept { return Flags{lhs}.flags_ == rhs.flags_; }
  friend constexpr bool operator!=(const Flags& lhs, const Flags& rhs) noexcept { return lhs.flags_ != rhs.flags_; }
  friend constexpr bool operator!=(const Flags& lhs, const T rhs) noexcept { return lhs.flags_ != Flags{rhs}.flags_; }
  friend constexpr bool operator!=(const T lhs, const Flags& rhs) noexcept { return Flags{lhs}.flags_ != rhs.flags_; }
  /// @}

private:
  constexpr explicit Flags(const underlying_type flags) noexcept : flags_{flags} {}

  underlying_type flags_;
};

} // namespace rtw::stl

template <typename T, typename = std::enable_if_t<rtw::stl::details::IS_SCOPED_ENUM_V<T>>>
constexpr rtw::stl::Flags<T> operator|(const T lhs, const T rhs) noexcept
{
  return rtw::stl::Flags<T>{lhs} | rhs;
}

template <typename T, typename = std::enable_if_t<rtw::stl::details::IS_SCOPED_ENUM_V<T>>>
constexpr rtw::stl::Flags<T> operator&(const T lhs, const T rhs) noexcept
{
  return rtw::stl::Flags<T>{lhs} & rhs;
}

template <typename T, typename = std::enable_if_t<rtw::stl::details::IS_SCOPED_ENUM_V<T>>>
constexpr rtw::stl::Flags<T> operator^(const T lhs, const T rhs) noexcept
{
  return rtw::stl::Flags<T>{lhs} ^ rhs;
}
