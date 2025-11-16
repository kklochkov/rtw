#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T>
class HeapArray
{
  using StorageType = ContiguousStorage<T>;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;

  explicit HeapArray(const size_type capacity) noexcept : storage_{capacity}
  {
    for (size_type i = 0; i < capacity; ++i)
    {
      storage_.construct_at(i);
    }
  }

  template <typename U = T>
  HeapArray(const size_type capacity, const U& initial_value) noexcept : storage_{capacity}
  {
    for (size_type i = 0; i < capacity; ++i)
    {
      storage_.construct_at(i, initial_value);
    }
  }

  size_type size() const noexcept { return storage_.used_slots(); }

  reference operator[](const size_type index) noexcept { return storage_[index]; }
  const_reference operator[](const size_type index) const noexcept { return storage_[index]; }

  iterator begin() noexcept { return storage_.begin(); }
  const_iterator begin() const noexcept { return storage_.begin(); }
  const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  iterator end() noexcept { return storage_.end(); }
  const_iterator end() const noexcept { return storage_.end(); }
  const_iterator cend() const noexcept { return storage_.cend(); }

private:
  StorageType storage_;
};

} // namespace rtw::stl
