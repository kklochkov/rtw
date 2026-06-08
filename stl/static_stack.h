#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

/// @brief A fixed-capacity LIFO stack.
///
/// GenericStaticStack provides a stack interface over a contiguous storage backend.
/// Never allocates dynamically when used with InplaceStaticContiguousStorage.
///
/// @tparam T Element type.
/// @tparam StorageT Storage backend.
///
/// Complexity:
///   - push / emplace: O(1)
///   - pop: O(1)
///   - top / bottom: O(1)
///   - size / empty / capacity: O(1)
template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticStack
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStaticStack() noexcept = default;
  constexpr explicit GenericStaticStack(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  /// @brief Constructs an element in-place on top of the stack.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the newly constructed element.
  /// @pre size() < capacity()
  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    assert(size() < capacity());
    return storage_.construct_at(size(), std::forward<ArgsT>(args)...);
  }

  /// @brief Pushes @p value onto the stack.
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
    value = top();
    pop();
  }

  /// @brief Removes the top element.
  /// @pre !empty()
  constexpr void pop() noexcept
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  /// @brief Returns a reference to the top (most recently pushed) element.
  /// @pre !empty()
  constexpr reference top() noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  /// @brief Returns a const reference to the top (most recently pushed) element.
  /// @pre !empty()
  constexpr const_reference top() const noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  /// @brief Returns a reference to the bottom (first pushed) element.
  /// @pre !empty()
  constexpr reference bottom() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Returns a const reference to the bottom (first pushed) element.
  /// @pre !empty()
  constexpr const_reference bottom() const noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Destroys all elements and sets size to zero.
  constexpr void clear() noexcept { storage_.clear(); }

private:
  StorageType storage_;
};

template <typename T>
using StaticStack = GenericStaticStack<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceStaticStack = GenericStaticStack<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
