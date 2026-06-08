#pragma once

#include "stl/static_contiguous_storage.h"

#include <functional>

namespace rtw::stl
{

/// @brief A fixed-capacity binary max-heap priority queue.
///
/// GenericStaticPriorityQueue maintains the heap property using a configurable comparator.
/// By default uses std::less, yielding a max-heap (top() returns the largest element).
/// Never allocates dynamically when used with InplaceStaticContiguousStorage.
///
/// @tparam T Element type.
/// @tparam CompareT Comparator (default: std::less<T>, giving a max-heap).
/// @tparam StorageT Storage backend.
///
/// Complexity:
///   - push / emplace: O(log n)
///   - pop: O(log n)
///   - top: O(1)
///   - size / empty / capacity: O(1)
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
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  /// @brief Constructs an element in-place and restores the heap property.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the element at its final heap position.
  /// @pre size() < capacity()
  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    assert(size() < capacity());
    storage_.construct_at(size(), std::forward<ArgsT>(args)...);
    const auto index = push_heap(size() - 1U);
    return storage_[index];
  }

  /// @brief Pushes @p value into the priority queue.
  /// @param[in] value Element to push.
  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  /// @brief Pops the top element, copying it into @p value.
  /// @param[out] value Destination for the popped element.
  /// @pre !empty()
  constexpr void pop(T& value) noexcept
  {
    value = storage_[0U];
    pop();
  }

  /// @brief Removes the top (highest-priority) element.
  /// @pre !empty()
  constexpr void pop() noexcept
  {
    assert(!empty());
    storage_[0U] = std::move(storage_[size() - 1U]);
    storage_.destruct_at(size() - 1U);
    pop_heap(0U);
  }

  /// @brief Returns a reference to the top (highest-priority) element.
  /// @pre !empty()
  constexpr reference top() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Returns a const reference to the top (highest-priority) element.
  /// @pre !empty()
  constexpr const_reference top() const noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Destroys all elements and sets size to zero.
  constexpr void clear() noexcept { storage_.clear(); }

private:
  constexpr size_type get_parent(const size_type index) const noexcept { return (index - 1U) / 2U; }
  constexpr size_type get_left(const size_type index) const noexcept { return (2U * index) + 1U; }
  constexpr size_type get_right(const size_type index) const noexcept { return (2U * index) + 2U; }

  constexpr void swap_elements(const size_type a, const size_type b) noexcept
  {
    auto tmp = std::move(storage_[a]);
    storage_[a] = std::move(storage_[b]);
    storage_[b] = std::move(tmp);
  }

  constexpr size_type push_heap(size_type index) noexcept
  {
    while ((index > 0U) && compare_(storage_[get_parent(index)], storage_[index]))
    {
      swap_elements(get_parent(index), index);
      index = get_parent(index);
    }
    return index;
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
      swap_elements(index, next_index);
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
