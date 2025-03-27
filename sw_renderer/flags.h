#pragma once

#include <iosfwd>
#include <type_traits>

namespace rtw::sw_renderer
{

template <typename T>
class Flags;

template <typename T>
std::ostream& operator<<(std::ostream& os, const Flags<T>& flags) noexcept;

template <typename T>
class Flags
{
public:
  static_assert(std::is_enum_v<T>, "T must be an enum type.");

  using underlying_type = std::underlying_type_t<T>;

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

  constexpr Flags operator|(const Flags& rhs) const noexcept
  {
    return Flags{static_cast<underlying_type>(flags_ | rhs.flags_)};
  }
  constexpr Flags operator|(const underlying_type rhs) const noexcept { return operator|(Flags{rhs}); }
  constexpr Flags operator|(const T rhs) const noexcept { return operator|(Flags{rhs}); }

  constexpr Flags operator&(const Flags& rhs) const noexcept
  {
    return Flags{static_cast<underlying_type>(flags_ & rhs.flags_)};
  }
  constexpr Flags operator&(const underlying_type rhs) const noexcept { return operator&(Flags{rhs}); }
  constexpr Flags operator&(const T rhs) const noexcept { return operator&(Flags{rhs}); }

  constexpr Flags operator^(const Flags& rhs) const noexcept
  {
    return Flags{static_cast<underlying_type>(flags_ ^ rhs.flags_)};
  }
  constexpr Flags operator^(const underlying_type rhs) const noexcept { return operator^(Flags{rhs}); }
  constexpr Flags operator^(const T rhs) const noexcept { return operator^(Flags{rhs}); }

  constexpr Flags operator~() const noexcept { return Flags{static_cast<underlying_type>(~flags_)}; }

  constexpr Flags& operator|=(const Flags& rhs) noexcept
  {
    flags_ = flags_ | rhs.flags_;
    return *this;
  }
  constexpr Flags& operator|=(const underlying_type rhs) noexcept { return operator|=(Flags{rhs}); }
  constexpr Flags& operator|=(const T rhs) noexcept { return operator|=(Flags{rhs}); }

  constexpr Flags& operator&=(const Flags& rhs) noexcept
  {
    flags_ = flags_ & rhs.flags_;
    return *this;
  }

  constexpr Flags& operator&=(const underlying_type rhs) noexcept { return operator&=(Flags{rhs}); }
  constexpr Flags& operator&=(const T rhs) noexcept { return operator&=(Flags{rhs}); }

  constexpr Flags& operator^=(const Flags& rhs) noexcept
  {
    flags_ = flags_ ^ rhs.flags_;
    return *this;
  }
  constexpr Flags& operator^=(const underlying_type rhs) noexcept { return operator^=(Flags{rhs}); }
  constexpr Flags& operator^=(const T rhs) noexcept { return operator^=(Flags{rhs}); }

  constexpr bool operator==(const Flags& rhs) const noexcept { return flags_ == rhs.flags_; }
  constexpr bool operator==(const underlying_type rhs) const noexcept { return operator==(Flags{rhs}); }
  constexpr bool operator==(const T rhs) const noexcept { return operator==(Flags{rhs}); }

  constexpr bool operator!=(const Flags& rhs) const noexcept { return flags_ != rhs.flags_; }
  constexpr bool operator!=(const underlying_type rhs) const noexcept { return operator!=(Flags{rhs}); }
  constexpr bool operator!=(const T rhs) const noexcept { return operator!=(Flags{rhs}); }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Flags operator|(const underlying_type lhs, const Flags& rhs) noexcept { return Flags{lhs} | rhs; }
  friend constexpr Flags operator|(const T lhs, const Flags& rhs) noexcept { return Flags{lhs} | rhs; }
  friend constexpr Flags operator&(const underlying_type lhs, const Flags& rhs) noexcept { return Flags{lhs} & rhs; }
  friend constexpr Flags operator&(const T lhs, const Flags& rhs) noexcept { return Flags{lhs} & rhs; }
  friend constexpr Flags operator^(const underlying_type lhs, const Flags& rhs) noexcept { return Flags{lhs} ^ rhs; }
  friend constexpr Flags operator^(const T lhs, const Flags& rhs) noexcept { return Flags{lhs} ^ rhs; }

  friend constexpr bool operator==(const underlying_type lhs, const Flags& rhs) noexcept { return lhs == rhs.flags_; }
  friend constexpr bool operator==(const T lhs, const Flags& rhs) noexcept { return Flags{lhs} == rhs.flags_; }
  friend constexpr bool operator!=(const underlying_type lhs, const Flags& rhs) noexcept { return lhs != rhs.flags_; }
  friend constexpr bool operator!=(const T lhs, const Flags& rhs) noexcept { return Flags{lhs} != rhs.flags_; }
  /// @}

  friend std::ostream& operator<< <T>(std::ostream& os, const Flags& flags) noexcept;

private:
  constexpr explicit Flags(const underlying_type flags) noexcept : flags_{flags} {}

  underlying_type flags_;
};

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr Flags<T> operator|(const T lhs, const T rhs) noexcept
{
  return Flags<T>{lhs} | Flags<T>{rhs};
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr Flags<T> operator&(const T lhs, const T rhs) noexcept
{
  return Flags<T>{lhs} & Flags<T>{rhs};
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr Flags<T> operator^(const T lhs, const T rhs) noexcept
{
  return Flags<T>{lhs} ^ Flags<T>{rhs};
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr Flags<T> operator~(const T lhs) noexcept
{
  return ~Flags<T>{lhs};
}

} // namespace rtw::sw_renderer
