#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

/// @brief A fixed-capacity FIFO queue.
///
/// GenericStaticQueue provides a queue interface over a contiguous storage backend.
/// Uses modular indexing with a head pointer; the tail position is computed as
/// (head + size) % capacity. Never allocates dynamically when used with
/// InplaceStaticContiguousStorage.
///
/// @tparam T Element type.
/// @tparam StorageT Storage backend.
///
/// Complexity:
///   - push / emplace: O(1)
///   - pop: O(1)
///   - front / back: O(1)
///   - size / empty / capacity: O(1)
template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticQueue
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStaticQueue() noexcept = default;
  constexpr explicit GenericStaticQueue(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  /// @brief Constructs an element in-place at the back of the queue.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the newly constructed element.
  /// @pre size() < capacity()
  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    assert(size() < capacity());
    const auto tail = (head_ + size()) % capacity();
    return storage_.construct_at(tail, std::forward<ArgsT>(args)...);
  }

  /// @brief Pushes @p value onto the back of the queue.
  /// @param[in] value Element to push.
  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  /// @brief Pops the front element, copying it into @p value.
  /// @param[out] value Destination for the popped element.
  /// @pre !empty()
  constexpr void pop(T& value) noexcept
  {
    assert(!empty());
    value = storage_[head_];
    pop();
  }

  /// @brief Removes the front element.
  /// @pre !empty()
  constexpr void pop() noexcept
  {
    assert(!empty());
    storage_.destruct_at(head_);
    head_ = (head_ + 1U) % capacity();
  }

  /// @brief Returns a reference to the front (oldest) element.
  /// @pre !empty()
  constexpr reference front() noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  /// @brief Returns a const reference to the front (oldest) element.
  /// @pre !empty()
  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  /// @brief Returns a reference to the back (newest) element.
  /// @pre !empty()
  constexpr reference back() noexcept
  {
    assert(!empty());
    return storage_[(head_ + size() - 1U) % capacity()];
  }

  /// @brief Returns a const reference to the back (newest) element.
  /// @pre !empty()
  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return storage_[(head_ + size() - 1U) % capacity()];
  }

  /// @brief Destroys all elements and resets the head index.
  constexpr void clear() noexcept
  {
    head_ = 0U;
    storage_.clear();
  }

private:
  StorageType storage_;
  size_type head_{0U};
};

template <typename T>
using StaticQueue = GenericStaticQueue<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceStaticQueue = GenericStaticQueue<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
