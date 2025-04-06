#pragma once

#include "sw_renderer/contiguous_storage.h"

namespace rtw::stl
{

template <typename T>
class CircularBuffer
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

  explicit CircularBuffer(const size_type capacity) : storage_{capacity} {}

  size_type size() const { return storage_.used_slots(); }
  bool empty() const { return storage_.empty(); }
  size_type capacity() const { return storage_.capacity(); }

  template <typename... ArgsT>
  reference emplace_back(ArgsT&&... args)
  {
    const auto index = (head_++) % capacity();
    if (storage_.is_constructed(index))
    {
      storage_.destruct_at(index);
    }
    return storage_.construct_at(index, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  void push_back(U&& value)
  {
    emplace_back(std::forward<U>(value));
  }

  void pop_back()
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  void clear()
  {
    head_ = 0U;
    storage_.clear();
  }

  reference operator[](const size_type index) { return storage_[index]; }
  const_reference operator[](const size_type index) const { return storage_[index]; }

  iterator begin() { return storage_.begin(); }
  const_iterator begin() const { return storage_.begin(); }
  const_iterator cbegin() const { return storage_.cbegin(); }

  iterator end() { return storage_.end(); }
  const_iterator end() const { return storage_.end(); }
  const_iterator cend() const { return storage_.cend(); }

private:
  StorageType storage_;
  size_type head_{0U};
};

} // namespace rtw::stl
