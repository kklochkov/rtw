#pragma once

#include "stl/static_contiguous_storage.h"

#include <functional>

namespace rtw::stl
{

template <typename T, typename CompareT, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticPriorityQueue
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using value_compare = CompareT;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStaticPriorityQueue() noexcept = default;
  constexpr explicit GenericStaticPriorityQueue(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    auto& result = storage_.construct_at(size(), std::forward<ArgsT>(args)...);
    push_heap(size() - 1U);
    return result;
  }

  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  constexpr void pop(T& value) noexcept
  {
    value = storage_[0U];
    pop();
  }

  constexpr void pop() noexcept
  {
    storage_[0U] = storage_[size() - 1U];
    storage_.destruct_at(size() - 1U);
    pop_heap(0U);
  }

  constexpr reference top() noexcept { return storage_[0U]; }
  constexpr const_reference top() const noexcept { return top(); }

  constexpr void clear() { storage_.clear(); }

private:
  constexpr size_type get_parent(const size_type index) const noexcept { return (index - 1U) / 2U; }
  constexpr size_type get_left(const size_type index) const noexcept { return (2U * index) + 1U; }
  constexpr size_type get_right(const size_type index) const noexcept { return (2U * index) + 2U; }

  constexpr void push_heap(size_type index) noexcept
  {
    while ((index > 0U) && compare_(storage_[get_parent(index)], storage_[index]))
    {
      std::swap(storage_[get_parent(index)], storage_[index]);
      index = get_parent(index);
    }
  }

  constexpr void pop_heap(const size_type index) noexcept
  {
    size_type next_index = index;
    const size_type left = get_left(index);
    const size_type right = get_right(index);

    if ((left < size()) && compare_(storage_[next_index], storage_[left]))
    {
      next_index = left;
    }

    if ((right < size()) && compare_(storage_[next_index], storage_[right]))
    {
      next_index = right;
    }

    if (next_index != index)
    {
      std::swap(storage_[index], storage_[next_index]);
      pop_heap(next_index);
    }
  }

  StorageType storage_;
  CompareT compare_;
};

template <typename T, typename CompareT = std::less<T>>
using StaticPriorityQueue = GenericStaticPriorityQueue<T, CompareT, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY, typename CompareT = std::less<T>>
using InplaceStaticPriorityQueue = GenericStaticPriorityQueue<T, CompareT, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
