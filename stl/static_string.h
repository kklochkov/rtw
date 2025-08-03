#pragma once

#include "stl/string_view.h"

#include <iosfwd>
#include <memory>

namespace rtw::stl
{

class StaticString
{
  using StorageType = std::unique_ptr<char[]>;

public:
  using value_type = char;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  constexpr static size_type NPOS = std::string::npos;

  explicit StaticString(const size_type capacity) noexcept
      : storage_{std::make_unique<char[]>(capacity + 1U)}, capacity_{capacity}
  {
    assert(capacity > 0U);
    ensure_null_termination();
  }

  StaticString(const_pointer str, const size_type capacity) noexcept : StaticString{capacity}
  {
    assert(str != nullptr);
    size_ = std::min(capacity, std::char_traits<char>::length(str));
    for (size_type i = 0U; i < size_; ++i)
    {
      storage_[i] = str[i];
    }
    ensure_null_termination();
  }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  StaticString(const_pointer str) noexcept : StaticString{str, std::char_traits<char>::length(str)} {}
  StaticString(StringView str) noexcept : StaticString{str.data(), str.size()} {}
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return size_ == 0U; }
  size_type capacity() const noexcept { return capacity_; }

  void clear() noexcept
  {
    size_ = 0U;
    ensure_null_termination();
  }

  void push_back(const value_type value) noexcept
  {
    if (size_ < capacity_)
    {
      storage_[size_] = value;
      ++size_;
      ensure_null_termination();
    }
  }

  void pop_back() noexcept
  {
    if (!empty())
    {
      --size_;
      ensure_null_termination();
    }
  }

  StaticString& operator+=(const StaticString& other) noexcept
  {
    const size_type size = std::min(capacity_ - size_, other.size_);
    for (size_type i = 0U; i < size; ++i)
    {
      storage_[size_ + i] = other.storage_[i];
    }
    size_ += size;
    ensure_null_termination();

    return *this;
  }

  StaticString& operator+=(const value_type value) noexcept
  {
    push_back(value);
    return *this;
  }

  reference operator[](const size_type index) noexcept
  {
    assert(index < size_);
    return storage_[index];
  }

  const_reference operator[](const size_type index) const noexcept
  {
    assert(index < size_);
    return storage_[index];
  }

  reference front() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  const_reference front() const noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  reference back() noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  const_reference back() const noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  iterator begin() noexcept { return &front(); }
  const_iterator begin() const noexcept { return &front(); }
  const_iterator cbegin() const noexcept { return &front(); }

  iterator end() noexcept
  {
    assert(!empty());
    return &storage_[capacity_];
  }

  const_iterator end() const noexcept
  {
    assert(!empty());
    return &storage_[capacity_];
  }

  const_iterator cend() const noexcept
  {
    assert(!empty());
    return &storage_[capacity_];
  }

  pointer data() noexcept { return begin(); }
  const_pointer data() const noexcept { return cbegin(); }
  const_pointer c_str() const noexcept { return cbegin(); }

  size_type find(const StaticString& str, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(str.data(), pos);
  }

  size_type find(const_pointer s, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(s, pos);
  }

  size_type find(const value_type c, const size_type pos = 0U) const noexcept
  {
    return StringView{data(), size()}.find(c, pos);
  }

  std::int32_t compare(const StaticString& other) const noexcept
  {
    return StringView{data(), size()}.compare(StringView{other.data(), other.size()});
  }

  std::int32_t compare(const_pointer s) const noexcept { return StringView{data(), size()}.compare(StringView{s}); }

  bool starts_with(const StaticString& prefix) const noexcept
  {
    return StringView{data(), size()}.starts_with(StringView{prefix.data(), prefix.size()});
  }

  bool starts_with(const_pointer s) const noexcept { return StringView{data(), size()}.starts_with(StringView{s}); }

  bool ends_with(const StaticString& suffix) const noexcept
  {
    return StringView{data(), size()}.ends_with(StringView{suffix.data(), suffix.size()});
  }

  bool ends_with(const_pointer s) const noexcept { return StringView{data(), size()}.ends_with(StringView{s}); }

  bool contains(const StaticString& str) const noexcept
  {
    return StringView{data(), size()}.contains(StringView{str.data(), str.size()});
  }
  bool contains(const_pointer s) const noexcept { return StringView{data(), size()}.contains(StringView{s}); }
  bool contains(const value_type c) const noexcept { return StringView{data(), size()}.contains(c); }

  StringView substr(const size_type pos = 0U, const size_type count = NPOS) const noexcept
  {
    return StringView{data(), size()}.substr(pos, count);
  }

  friend bool operator==(const StaticString& lhs, const StaticString& rhs) noexcept { return lhs.compare(rhs) == 0; }
  friend bool operator!=(const StaticString& lhs, const StaticString& rhs) noexcept { return !(lhs == rhs); }
  friend bool operator<(const StaticString& lhs, const StaticString& rhs) noexcept { return lhs.compare(rhs) < 0; }
  friend bool operator<=(const StaticString& lhs, const StaticString& rhs) noexcept { return lhs.compare(rhs) <= 0; }
  friend bool operator>(const StaticString& lhs, const StaticString& rhs) noexcept { return lhs.compare(rhs) > 0; }
  friend bool operator>=(const StaticString& lhs, const StaticString& rhs) noexcept { return lhs.compare(rhs) >= 0; }

  friend std::ostream& operator<<(std::ostream& os, const StaticString& string) noexcept;

private:
  void ensure_null_termination() noexcept
  {
    storage_[size_] = '\0'; // Ensure null-termination
  }

  StorageType storage_;
  size_type size_{0U};
  size_type capacity_{0U};
};

} // namespace rtw::stl
