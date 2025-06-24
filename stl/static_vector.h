#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T>
class StaticVector
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

  explicit StaticVector(const size_type capacity) noexcept : storage_{capacity} {}

  size_type size() const noexcept { return storage_.used_slots(); }
  bool empty() const noexcept { return storage_.empty(); }
  size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  reference emplace_back(ArgsT&&... args) noexcept
  {
    return storage_.construct_at(size(), std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  void pop_back() noexcept
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  void clear() noexcept { storage_.clear(); }

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
