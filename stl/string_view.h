#pragma once

#include "stl/span.h"

#include <string>

namespace rtw::stl
{

class StringView
{
  using StorageType = Span<const char>;

public:
  using element_type = typename StorageType::element_type;
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using difference_type = typename StorageType::difference_type;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;

  constexpr StringView() noexcept = default;
  constexpr StringView(const_pointer str, const size_type size) noexcept : storage_{str, size} {}
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr StringView(const_pointer str) noexcept : storage_{str, std::char_traits<char>::length(str)} {}

  constexpr size_type size() const noexcept { return storage_.size(); }
  constexpr size_type length() const noexcept { return size(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }

  constexpr const_pointer data() const noexcept { return storage_.data(); }
  constexpr const_pointer c_str() const noexcept { return storage_.data(); }
  constexpr const_reference operator[](const size_type index) const noexcept { return storage_[index]; }

  constexpr const_reference front() const noexcept { return storage_.front(); }
  constexpr const_reference back() const noexcept { return storage_.back(); }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.end(); }

  constexpr void remove_prefix(const size_type n) noexcept
  {
    assert(n <= size());
    storage_ = storage_.subspan(n, size() - n);
  }

  constexpr void remove_suffix(const size_type n) noexcept
  {
    assert(n <= size());
    storage_ = storage_.subspan(0U, size() - n);
  }

  constexpr size_type copy(char* dest, const size_type count, const size_type pos = 0U) const noexcept
  {
    const size_type length = std::min(count, size() - pos);
    std::copy_n(storage_.data() + pos, length, dest);
    return length;
  }

  constexpr StringView substr(const size_type pos = 0U, const size_type count = std::string::npos) const noexcept
  {
    const size_type length = std::min(count, size() - pos);
    return StringView{storage_.data() + pos, length};
  }

  constexpr std::int32_t compare(const StringView other) const noexcept
  {
    const size_type lhs = size();
    const size_type rhs = other.size();
    const size_type min_size = std::min(size(), other.size());
    const std::int32_t cmp = std::char_traits<char>::compare(cbegin(), other.cbegin(), min_size);
    if (cmp != 0)
    {
      return cmp;
    }

    if (lhs < rhs)
    {
      return -1; // This string is less than the other
    }

    if (lhs > rhs)
    {
      return 1; // This string is greater than the other
    }

    return 0; // Strings are equal
  }

  constexpr std::int32_t compare(const_pointer str) const noexcept { return compare(StringView{str}); }

  constexpr bool starts_with(const StringView prefix) const noexcept
  {
    if (prefix.size() > size())
    {
      return false; // Prefix is longer than the string
    }
    return std::char_traits<char>::compare(cbegin(), prefix.cbegin(), prefix.size()) == 0;
  }

  constexpr bool starts_with(const_pointer prefix) const noexcept { return starts_with(StringView{prefix}); }

  constexpr bool ends_with(const StringView suffix) const noexcept
  {
    if (suffix.size() > size())
    {
      return false; // Suffix is longer than the string
    }
    return std::char_traits<char>::compare(cend() - suffix.size(), suffix.cbegin(), suffix.size()) == 0;
  }

  constexpr bool ends_with(const_pointer suffix) const noexcept { return ends_with(StringView{suffix}); }

  constexpr size_type find(const StringView str, const size_type pos = 0U) const noexcept
  {
    if (pos >= size() || str.empty() || size() < str.size())
    {
      return std::string::npos;
    }

    for (size_type i = pos; i <= size() - str.size(); ++i)
    {
      if (std::equal(cbegin() + i, cbegin() + i + str.size(), str.cbegin()))
      {
        return i;
      }
    }

    return std::string::npos;
  }

  constexpr size_type find(const_pointer s, const size_type pos = 0U) const noexcept
  {
    return find(StringView{s}, pos);
  }

  constexpr size_type find(const value_type c, const size_type pos = 0U) const noexcept
  {
    if (pos >= size())
    {
      return std::string::npos;
    }

    for (size_type i = pos; i < size(); ++i)
    {
      if (storage_[i] == c)
      {
        return i;
      }
    }

    return std::string::npos;
  }

  constexpr bool contains(const StringView str) const noexcept { return find(str) != std::string::npos; }
  constexpr bool contains(const_pointer str) const noexcept { return find(str) != std::string::npos; }
  constexpr bool contains(const value_type c) const noexcept { return find(c) != std::string::npos; }

  friend constexpr bool operator==(const StringView lhs, const StringView rhs) noexcept
  {
    return lhs.compare(rhs) == 0;
  }

  friend constexpr bool operator!=(const StringView lhs, const StringView rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const StringView lhs, const StringView rhs) noexcept { return lhs.compare(rhs) < 0; }
  friend constexpr bool operator<=(const StringView lhs, const StringView rhs) noexcept
  {
    return lhs.compare(rhs) <= 0;
  }
  friend constexpr bool operator>(const StringView lhs, const StringView rhs) noexcept { return lhs.compare(rhs) > 0; }
  friend constexpr bool operator>=(const StringView lhs, const StringView rhs) noexcept
  {
    return lhs.compare(rhs) >= 0;
  }

private:
  StorageType storage_;
};

} // namespace rtw::stl
