#pragma once

#include "stl/string_view.h"

#include <iosfwd>
#include <memory>

namespace rtw::stl
{

template <typename DerivedT>
class GenericStaticString;

template <typename DerivedT>
std::ostream& operator<<(std::ostream& os, const GenericStaticString<DerivedT>& string) noexcept;

template <typename DerivedT>
class GenericStaticString
{
  friend DerivedT;

public:
  using value_type = char;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;

  constexpr size_type size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0U; }
  constexpr size_type capacity() const noexcept { return capacity_; }

  constexpr void clear() noexcept
  {
    size_ = 0U;
    ensure_null_termination();
  }

  constexpr void push_back(const value_type value) noexcept
  {
    if (size_ < capacity_)
    {
      get_derived().value(size_) = value;
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

  constexpr GenericStaticString& operator+=(const StringView& other) noexcept
  {
    const size_type size = std::min(capacity_ - size_, other.size());
    for (size_type i = 0U; i < size; ++i)
    {
      get_derived().value(size_ + i) = other[i];
    }
    size_ += size;
    ensure_null_termination();

    return *this;
  }

  constexpr GenericStaticString& operator+=(const GenericStaticString& other) noexcept
  {
    return *this += StringView{other.data(), other.size()};
  }

  constexpr GenericStaticString& operator+=(const value_type value) noexcept
  {
    push_back(value);
    return *this;
  }

  constexpr reference operator[](const size_type index) noexcept
  {
    assert(index < size_);
    return get_derived().value(index);
  }

  constexpr const_reference operator[](const size_type index) const noexcept
  {
    assert(index < size_);
    return get_derived().value(index);
  }

  constexpr reference front() noexcept
  {
    assert(!empty());
    return get_derived().value(0U);
  }

  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return get_derived().value(0U);
  }

  constexpr reference back() noexcept
  {
    assert(!empty());
    return get_derived().value(size() - 1U);
  }

  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return get_derived().value(size() - 1U);
  }

  constexpr iterator begin() noexcept { return get_derived().get_data(); }
  constexpr const_iterator begin() const noexcept { return get_derived().get_data(); }
  constexpr const_iterator cbegin() const noexcept { return get_derived().get_data(); }

  constexpr iterator end() noexcept { return begin() + size_; }
  constexpr const_iterator end() const noexcept { return begin() + size_; }
  constexpr const_iterator cend() const noexcept { return cbegin() + size_; }

  constexpr pointer data() noexcept { return begin(); }
  constexpr const_pointer data() const noexcept { return cbegin(); }
  constexpr const_pointer c_str() const noexcept { return cbegin(); }

  constexpr size_type find(const GenericStaticString& str, const size_type pos = 0U) const noexcept
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

  constexpr std::int32_t compare(const GenericStaticString& other) const noexcept
  {
    return StringView{data(), size()}.compare(StringView{other.data(), other.size()});
  }

  constexpr std::int32_t compare(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.compare(StringView{s});
  }

  constexpr bool starts_with(const GenericStaticString& prefix) const noexcept
  {
    return StringView{data(), size()}.starts_with(StringView{prefix.data(), prefix.size()});
  }

  constexpr bool starts_with(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.starts_with(StringView{s});
  }

  constexpr bool ends_with(const GenericStaticString& suffix) const noexcept
  {
    return StringView{data(), size()}.ends_with(StringView{suffix.data(), suffix.size()});
  }

  constexpr bool ends_with(const_pointer s) const noexcept
  {
    return StringView{data(), size()}.ends_with(StringView{s});
  }

  constexpr bool contains(const GenericStaticString& str) const noexcept
  {
    return StringView{data(), size()}.contains(StringView{str.data(), str.size()});
  }
  constexpr bool contains(const_pointer s) const noexcept { return StringView{data(), size()}.contains(StringView{s}); }
  constexpr bool contains(const value_type c) const noexcept { return StringView{data(), size()}.contains(c); }

  constexpr StringView substr(const size_type pos = 0U, const size_type count = std::string::npos) const noexcept
  {
    return StringView{data(), size()}.substr(pos, count);
  }

  friend constexpr bool operator==(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return lhs.compare(rhs) == 0;
  }
  friend constexpr bool operator!=(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return !(lhs == rhs);
  }
  friend constexpr bool operator<(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }
  friend constexpr bool operator<=(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return lhs.compare(rhs) <= 0;
  }
  friend constexpr bool operator>(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return lhs.compare(rhs) > 0;
  }
  friend constexpr bool operator>=(const GenericStaticString& lhs, const GenericStaticString& rhs) noexcept
  {
    return lhs.compare(rhs) >= 0;
  }
  friend constexpr bool operator==(const GenericStaticString& lhs, StringView& rhs) noexcept
  {
    return lhs.compare(rhs) == 0;
  }
  friend constexpr bool operator!=(const GenericStaticString& lhs, StringView& rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const GenericStaticString& lhs, StringView& rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }
  friend constexpr bool operator<=(const GenericStaticString& lhs, StringView& rhs) noexcept
  {
    return lhs.compare(rhs) <= 0;
  }
  friend constexpr bool operator>(const GenericStaticString& lhs, StringView& rhs) noexcept
  {
    return lhs.compare(rhs) > 0;
  }
  friend constexpr bool operator>=(const GenericStaticString& lhs, StringView& rhs) noexcept
  {
    return lhs.compare(rhs) >= 0;
  }
  friend constexpr bool operator==(StringView lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) == 0;
  }
  friend constexpr bool operator!=(StringView lhs, const GenericStaticString& rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(StringView lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) > 0;
  }
  friend constexpr bool operator<=(StringView lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) >= 0;
  }
  friend constexpr bool operator>(StringView lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) < 0;
  }
  friend constexpr bool operator>=(StringView lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) <= 0;
  }
  friend constexpr bool operator==(const GenericStaticString& lhs, const_pointer rhs) noexcept
  {
    return lhs.compare(rhs) == 0;
  }
  friend constexpr bool operator!=(const GenericStaticString& lhs, const_pointer rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const GenericStaticString& lhs, const_pointer rhs) noexcept
  {
    return lhs.compare(rhs) < 0;
  }
  friend constexpr bool operator<=(const GenericStaticString& lhs, const_pointer rhs) noexcept
  {
    return lhs.compare(rhs) <= 0;
  }
  friend constexpr bool operator>(const GenericStaticString& lhs, const_pointer rhs) noexcept
  {
    return lhs.compare(rhs) > 0;
  }
  friend constexpr bool operator>=(const GenericStaticString& lhs, const_pointer rhs) noexcept
  {
    return lhs.compare(rhs) >= 0;
  }
  friend constexpr bool operator==(const_pointer lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) == 0;
  }
  friend constexpr bool operator!=(const_pointer lhs, const GenericStaticString& rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const_pointer lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) > 0;
  }
  friend constexpr bool operator<=(const_pointer lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) >= 0;
  }
  friend constexpr bool operator>(const_pointer lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) < 0;
  }
  friend constexpr bool operator>=(const_pointer lhs, const GenericStaticString& rhs) noexcept
  {
    return rhs.compare(lhs) <= 0;
  }

  friend std::ostream& operator<< <DerivedT>(std::ostream& os, const GenericStaticString& string) noexcept;

private:
  constexpr GenericStaticString() noexcept = default;

  constexpr DerivedT& get_derived() noexcept { return static_cast<DerivedT&>(*this); }
  constexpr const DerivedT& get_derived() const noexcept { return static_cast<const DerivedT&>(*this); }

  constexpr void init_from_string_view(StringView str)
  {
    const auto size = std::min(capacity_, str.size());
    auto& derived = get_derived();
    for (size_type i = 0U; i < size; ++i)
    {
      derived.value(i) = str[i];
    }
    ensure_null_termination();
  }

  constexpr void ensure_null_termination() noexcept { get_derived().value(size_) = '\0'; }

  size_type size_{0U};
  size_type capacity_{0U};
};

class StaticString : public GenericStaticString<StaticString>
{
  using StorageType = std::unique_ptr<char[]>;
  using Base = GenericStaticString<StaticString>;

  friend Base;

public:
  // Default constructor creates an empty StaticString with a capacity of 0.
  // It is used to move-construct an empty StaticString.
  StaticString() noexcept = default;

  explicit StaticString(const size_type capacity) noexcept : storage_{std::make_unique<char[]>(capacity + 1U)}
  {
    assert(capacity > 0U);
    capacity_ = capacity;
    ensure_null_termination();
  }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  StaticString(StringView str) noexcept : StaticString{str.size()}
  {
    size_ = str.size();
    init_from_string_view(str);
  }
  StaticString(const_pointer str) noexcept : StaticString{StringView{str}} {}
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  StaticString(const StaticString& other) noexcept : StaticString{StringView{other.data(), other.size()}} {}
  StaticString(StaticString&& other) noexcept : storage_{std::move(other.storage_)}
  {
    size_ = other.size_;
    capacity_ = other.capacity_;
    other = StaticString{};
  }
  StaticString& operator=(const StaticString& other) noexcept
  {
    if (this != &other)
    {
      size_ = std::min(capacity_, other.size_);
      for (size_type i = 0U; i < size_; ++i)
      {
        storage_[i] = other.storage_[i];
      }
      ensure_null_termination();
    }
    return *this;
  }
  StaticString& operator=(StaticString&& other) noexcept
  {
    if (this != &other)
    {
      storage_ = std::move(other.storage_);
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.size_ = 0U;
      other.capacity_ = 0U;
      other.storage_ = nullptr;
    }
    return *this;
  }
  ~StaticString() noexcept = default;

private:
  reference value(const size_type index) noexcept { return storage_[index]; }
  const_reference value(const size_type index) const noexcept { return storage_[index]; }

  pointer get_data() noexcept { return storage_.get(); }
  const_pointer get_data() const noexcept { return storage_.get(); }

  StorageType storage_{nullptr};
};

template <std::size_t CAPACITY>
class InplaceString : public GenericStaticString<InplaceString<CAPACITY>>
{
  using StorageType = std::array<char, CAPACITY + 1U>;
  using Base = GenericStaticString<InplaceString<CAPACITY>>;

  friend Base;

public:
  using value_type = typename Base::value_type;
  using size_type = typename Base::size_type;
  using reference = typename Base::reference;
  using const_reference = typename Base::const_reference;
  using pointer = typename Base::pointer;
  using const_pointer = typename Base::const_pointer;
  using iterator = typename Base::iterator;
  using const_iterator = typename Base::const_iterator;

  constexpr InplaceString() noexcept
  {
    Base::capacity_ = CAPACITY;
    Base::ensure_null_termination();
  }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr InplaceString(StringView str) noexcept
  {
    Base::capacity_ = CAPACITY;
    Base::size_ = std::min(Base::capacity(), str.size());
    for (size_type i = 0U; i < Base::size_; ++i)
    {
      storage_[i] = str[i];
    }
    Base::ensure_null_termination();
  }

  constexpr InplaceString(const_pointer str) noexcept : InplaceString{StringView{str}} {}
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

private:
  constexpr reference value(const size_type index) noexcept { return storage_[index]; }
  constexpr const_reference value(const size_type index) const noexcept { return storage_[index]; }

  constexpr pointer get_data() noexcept { return storage_.data(); }
  constexpr const_pointer get_data() const noexcept { return storage_.data(); }

  StorageType storage_;
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

template <>
struct std::hash<rtw::stl::StaticString>
{
  std::size_t operator()(const rtw::stl::StaticString& str) const noexcept
  {
    return std::hash<std::string_view>{}(std::string_view{str.data(), str.size()});
  }
};

template <std::size_t CAPACITY>
struct std::hash<rtw::stl::InplaceString<CAPACITY>>
{
  std::size_t operator()(const rtw::stl::InplaceString<CAPACITY>& str) const noexcept
  {
    return std::hash<std::string_view>{}(std::string_view{str.data(), str.size()});
  }
};
