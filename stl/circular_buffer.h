#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericCircularBuffer
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;

  constexpr GenericCircularBuffer() noexcept = default;
  constexpr explicit GenericCircularBuffer(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_front(ArgsT&&... args) noexcept
  {
    head_ = (head_ + capacity() - 1U) % capacity();
    return emplace_at_index(head_, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push_front(U&& value) noexcept
  {
    emplace_front(std::forward<U>(value));
  }

  constexpr void pop_front() noexcept
  {
    advance_head();
    storage_.destruct_at(head_);
  }

  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    const auto index = (head_ + size()) % capacity();

    if (size() == capacity())
    {
      // Start overwrite old data.
      advance_head();
    }

    return emplace_at_index(index, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  constexpr void pop_back() noexcept { storage_.destruct_at(get_back_index()); }

  constexpr void clear() noexcept
  {
    head_ = 0U;
    storage_.clear();
  }

  constexpr reference front() noexcept { return storage_[head_]; }
  constexpr const_reference front() const noexcept { return storage_[head_]; }

  constexpr reference back() noexcept { return storage_[get_back_index()]; }
  constexpr const_reference back() const noexcept { return storage_[get_back_index()]; }

  constexpr reference operator[](const size_type index) noexcept { return storage_[get_index(index)]; }
  constexpr const_reference operator[](const size_type index) const noexcept { return storage_[get_index(index)]; }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

private:
  constexpr void advance_head() noexcept { head_ = (head_ + 1U) % capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_at_index(const size_type index, ArgsT&&... args) noexcept
  {
    if (storage_.is_constructed(index))
    {
      storage_.destruct_at(index);
    }
    return storage_.construct_at(index, std::forward<ArgsT>(args)...);
  }

  constexpr size_type get_index(const size_type index) const noexcept { return (head_ + index) % capacity(); }
  constexpr size_type get_back_index() const noexcept { return (head_ + size() - 1U) % capacity(); }

  StorageType storage_;
  size_type head_{0U};
};

template <typename T>
using CircularBuffer = GenericCircularBuffer<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceCircularBuffer = GenericCircularBuffer<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
