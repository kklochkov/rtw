#pragma once

#include "stl/inplace_string.h"

#include <memory>

namespace rtw::stl
{

/// @brief A heap-allocated, fixed-capacity, null-terminated string with value semantics.
///
/// StaticString allocates its character buffer on the heap via std::unique_ptr. The capacity is
/// fixed at construction time. Use InplaceString for a fully constexpr, stack-allocated alternative.
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

} // namespace rtw::stl

template <>
struct std::hash<rtw::stl::StaticString>
{
  std::size_t operator()(const rtw::stl::StaticString& str) const noexcept
  {
    return std::hash<std::string_view>{}(std::string_view{str.data(), str.size()});
  }
};
