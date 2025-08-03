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
template <typename T, typename StorageT = ContiguousStorage<T>>
class GenericPackedBuffer
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

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    return storage_.construct_at(size(), std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  constexpr void pop_back() noexcept
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  constexpr void remove(const size_type index) noexcept
  {
    assert(index < size());
    const auto last_index = size() - 1U;
    std::exchange(storage_[index], storage_[last_index]);
    storage_.destruct_at(last_index);
  }

  constexpr void clear() noexcept { storage_.clear(); }

  constexpr reference operator[](const size_type index) noexcept { return storage_[index]; }
  constexpr const_reference operator[](const size_type index) const noexcept { return storage_[index]; }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

protected:
  constexpr explicit GenericPackedBuffer(const size_type capacity) noexcept : storage_{capacity} {}

private:
  StorageType storage_;
};

template <typename T>
class PackedBuffer : public GenericPackedBuffer<T, ContiguousStorage<T>>
{
  using Base = GenericPackedBuffer<T, ContiguousStorage<T>>;

public:
  using size_type = typename Base::size_type;

  explicit PackedBuffer(const size_type capacity) noexcept : Base{capacity} {}
};

template <typename T, std::size_t CAPACITY>
class InplacePackedBuffer : public GenericPackedBuffer<T, InplaceContiguousStorage<T, CAPACITY>>
{
  using Base = GenericPackedBuffer<T, InplaceContiguousStorage<T, CAPACITY>>;

public:
  constexpr InplacePackedBuffer() noexcept : Base{CAPACITY} {}
};

} // namespace rtw::stl
