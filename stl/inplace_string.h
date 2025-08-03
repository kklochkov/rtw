#pragma once

#include "stl/string_view.h"

#include <array>
#include <iosfwd>

namespace rtw::stl
{

template <std::size_t CAPACITY>
class InplaceString;

template <std::size_t CAPACITY>
std::ostream& operator<<(std::ostream& os, const InplaceString<CAPACITY>& string) noexcept;

template <std::size_t CAPACITY>
class InplaceString
{
  using StorageType = std::array<char, CAPACITY + 1U>;

public:
  using value_type = char;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;

  constexpr InplaceString() noexcept { ensure_null_termination(); }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr InplaceString(StringView str) noexcept : size_{std::min(capacity(), str.size())}
  {
    for (size_type i = 0U; i < size_; ++i)
    {
      storage_[i] = str[i];
    }
    ensure_null_termination();
  }

  constexpr InplaceString(const_pointer str) noexcept : InplaceString{StringView{str}} {}
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr size_type size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0U; }
  constexpr size_type capacity() const noexcept { return CAPACITY; }

  constexpr void clear() noexcept
  {
    size_ = 0U;
    ensure_null_termination();
  }

  constexpr void push_back(const value_type value) noexcept
  {
    if (size_ < capacity())
    {
      storage_[size_] = value;
      ++size_;
      ensure_null_termination();
    }
  }

  constexpr void pop_back() noexcept
  {
    if (!empty())
    {
      --size_;
      ensure_null_termination();
    }
  }

  constexpr InplaceString& operator+=(const InplaceString& other) noexcept
  {
    const size_type size = std::min(capacity() - size_, other.size_);
    for (size_type i = 0U; i < size; ++i)
    {
      storage_[size_ + i] = other.storage_[i];
    }
    size_ += size;
    ensure_null_termination();

    return *this;
  }

  constexpr InplaceString& operator+=(const value_type value) noexcept
  {
    push_back(value);
    return *this;
  }

  constexpr reference operator[](const size_type index) noexcept
  {
    assert(index < size_);
    return storage_[index];
  }

  constexpr const_reference operator[](const size_type index) const noexcept
  {
    assert(index < size_);
    return storage_[index];
  }

  constexpr reference front() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  constexpr reference back() noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  constexpr iterator begin() noexcept { return storage_.data(); }
  constexpr const_iterator begin() const noexcept { return storage_.data(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.data(); }

  constexpr iterator end() noexcept { return storage_.data() + capacity(); }
  constexpr const_iterator end() const noexcept { return storage_.data() + capacity(); }
  constexpr const_iterator cend() const noexcept { return storage_.data() + capacity(); }

  constexpr pointer data() noexcept { return storage_.data(); }
  constexpr const_pointer data() const noexcept { return storage_.data(); }
  constexpr const_pointer c_str() const noexcept { return storage_.data(); }

  constexpr size_type find(const InplaceString& str, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(str.data(), pos);
  }

  constexpr size_type find(const_pointer s, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(s, pos);
  }

  constexpr size_type find(const value_type c, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(c, pos);
  }

  constexpr std::int32_t compare(const InplaceString& other) const noexcept
  {
    return StringView{data(), size()}.compare(StringView{other.data(), other.size()});
  }

  constexpr std::int32_t compare(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.compare(StringView{s});
  }

  constexpr bool starts_with(const InplaceString& prefix) const noexcept
  {
    return StringView{data(), size()}.starts_with(StringView{prefix.data(), prefix.size()});
  }

  constexpr bool starts_with(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.starts_with(StringView{s});
  }

  constexpr bool ends_with(const InplaceString& suffix) const noexcept
  {
    return StringView{data(), size()}.ends_with(StringView{suffix.data(), suffix.size()});
  }

  constexpr bool ends_with(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.ends_with(StringView{s});
  }

  constexpr bool contains(const InplaceString& str) const noexcept
  {
    return StringView{data(), size()}.contains(StringView{str.data(), str.size()});
  }
  constexpr bool contains(const_pointer s) const noexcept { return StringView{data(), size()}.contains(StringView{s}); }
  constexpr bool contains(const value_type c) const noexcept { return StringView{data(), size()}.contains(c); }

  constexpr StringView substr(const size_type pos = 0U, const size_type count = std::string::npos) const noexcept
  {
    return StringView{data(), size()}.substr(pos, count);
  }

  friend constexpr bool operator==(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return lhs.compare(rhs) == 0;
  }

  friend constexpr bool operator!=(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return !(lhs == rhs);
  }

  friend constexpr bool operator<(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }

  friend constexpr bool operator<=(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return lhs.compare(rhs) <= 0;
  }

  friend constexpr bool operator>(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return lhs.compare(rhs) > 0;
  }

  friend constexpr bool operator>=(const InplaceString& lhs, const InplaceString& rhs) noexcept
  {
    return lhs.compare(rhs) >= 0;
  }

  friend std::ostream& operator<< <CAPACITY>(std::ostream& os, const InplaceString& string) noexcept;

private:
  constexpr void ensure_null_termination() noexcept { storage_[size_] = '\0'; }

  StorageType storage_;
  size_type size_{0U};
};

using InplaceStringSmall = InplaceString<32U>;
using InplaceStringMedium = InplaceString<64U>;
using InplaceStringLarge = InplaceString<128U>;
using InplaceStringXLarge = InplaceString<256U>;

template <std::size_t CAPACITY>
InplaceString(const char (&str)[CAPACITY]) -> InplaceString<CAPACITY - 1U>; // -1 to remove null terminator

template <std::size_t CAPACITY>
InplaceString<CAPACITY - 1U> make_string(const char (&str)[CAPACITY]) // -1 to remove null terminator
{
  return InplaceString<CAPACITY - 1U>{StringView{str}};
}

} // namespace rtw::stl

template <std::size_t CAPACITY>
struct std::hash<rtw::stl::InplaceString<CAPACITY>>
{
  std::size_t operator()(const rtw::stl::InplaceString<CAPACITY>& str) const noexcept
  {
    return std::hash<std::string_view>{}(std::string_view{str.data(), str.size()});
  }
};
