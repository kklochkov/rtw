#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

/// Class keeps instances of T in a contiguous memory block tightly packed.
/// Avoiding memory fragmentation is achieve by reserving a fixed size of memory.
/// Upon removing an element, the last element is moved to the removed element's position.
/// For example, if the buffer contains 5 elements and the 3rd element is removed,
/// the 5th element is moved to the 3rd position.
///
/// Case 1, removing from the head of the buffer:
/// a = {1, 2, 3, 4, 5}
/// - remove(0) => a = {5, 2, 3, 4}
/// - remove(0) => a = {4, 2, 3}
/// - remove(0) => a = {3, 2}
/// - remove(0) => a = {2}
/// - remove(0) => a = {}
///
/// Case 2, removing from the tail of the buffer:
/// a = {1, 2, 3, 4, 5}
/// - remove(4) => a = {1, 2, 3, 4}
/// - remove(3) => a = {1, 2, 3}
/// - remove(2) => a = {1, 2}
/// - remove(1) => a = {1}
/// - remove(0) => a = {}
///
/// Case 3, removing from the middle of the buffer:
/// a = {1, 2, 3, 4, 5}
/// - remove(2) => a = {1, 2, 5, 4}
/// - remove(2) => a = {1, 2, 4}
/// - remove(1) => a = {1, 4}
/// - remove(0) => a = {4}
/// - remove(0) => a = {}
///
/// @tparam T Type of the element to be stored in the buffer.
template <typename T>
class PackedBuffer
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

  explicit PackedBuffer(const size_type capacity)  noexcept : storage_{capacity} {}

  size_type size() const  noexcept { return storage_.used_slots(); }
  bool empty() const  noexcept { return storage_.empty(); }
  size_type capacity() const  noexcept { return storage_.capacity(); }

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

  void remove(const typename ContiguousStorage<T>::size_type index) noexcept
  {
    assert(index < size());
    const auto last_index = size() - 1U;
    std::exchange(storage_[index], storage_[last_index]);
    storage_.destruct_at(last_index);
  }

  void clear()  noexcept { storage_.clear(); }

  reference operator[](const size_type index)  noexcept { return storage_[index]; }
  const_reference operator[](const size_type index) const  noexcept { return storage_[index]; }

  iterator begin()  noexcept { return storage_.begin(); }
  const_iterator begin() const  noexcept { return storage_.begin(); }
  const_iterator cbegin() const  noexcept { return storage_.cbegin(); }

  iterator end()  noexcept { return storage_.end(); }
  const_iterator end() const  noexcept { return storage_.end(); }
  const_iterator cend() const  noexcept { return storage_.cend(); }

private:
  StorageType storage_;
};

} // namespace rtw::stl
