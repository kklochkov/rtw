#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

/// @brief A fixed-capacity vector with contiguous element storage.
///
/// GenericStaticVector provides a std::vector-like interface over a fixed-capacity storage backend.
/// Elements are stored contiguously and the container never allocates dynamically (when used with
/// InplaceStaticContiguousStorage). Supports both index-based and iterator-based insert/erase.
///
/// @note Iterator invalidation:
///   - insert(): invalidates all iterators at or after the insertion point.
///   - erase(): invalidates all iterators at or after the erased position.
///   - push_back() / emplace_back(): invalidates only end().
///   - pop_back(): invalidates end() and the iterator to the last element.
///   - clear() / resize(): invalidates all iterators.
///
/// @tparam T Element type.
/// @tparam StorageT Storage backend (StaticContiguousStorage for heap, InplaceStaticContiguousStorage for embedded).
///
/// Complexity:
///   - push_back / emplace_back: O(1) amortized
///   - pop_back: O(1)
///   - insert: O(n) where n = size() - position
///   - erase: O(n) where n = size() - position
///   - operator[]: O(1)
///   - size / empty / capacity: O(1)
template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticVector
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
  using reverse_iterator = typename StorageType::reverse_iterator;
  using const_reverse_iterator = typename StorageType::const_reverse_iterator;

  constexpr GenericStaticVector() noexcept = default;
  constexpr explicit GenericStaticVector(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  /// @brief Constructs an element in-place at the end of the vector.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the newly constructed element.
  /// @pre size() < capacity()
  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    assert(size() < capacity());
    return storage_.construct_at(size(), std::forward<ArgsT>(args)...);
  }

  /// @brief Appends @p value to the end of the vector.
  /// @param[in] value Element to append.
  template <typename U = T>
  constexpr void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  /// @brief Removes the last element.
  /// @pre !empty()
  constexpr void pop_back() noexcept
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  /// @brief Resizes the vector to contain @p count elements.
  /// If count > size(), default-constructed elements are appended.
  /// If count < size(), elements are removed from the back.
  /// @param[in] count Target number of elements.
  constexpr void resize(const size_type count) noexcept
  {
    assert(count <= capacity());
    while (size() > count)
    {
      pop_back();
    }
    while (size() < count)
    {
      emplace_back();
    }
  }

  /// @brief Inserts @p value at the given @p index, shifting subsequent elements to the right.
  /// @param[in] index Position at which to insert.
  /// @param[in] value Element to insert.
  /// @return Iterator to the inserted element.
  /// @pre index <= size()
  /// @pre size() < capacity()
  constexpr iterator insert(const size_type index, const value_type& value) noexcept
  {
    assert(index <= size());
    assert(size() < capacity());
    storage_.construct_for_overwrite_at(size());
    for (size_type i = size() - 1U; i > index; --i)
    {
      storage_[i] = std::move(storage_[i - 1U]);
    }
    storage_[index] = value;
    return begin() + static_cast<typename iterator::difference_type>(index);
  }

  /// @brief Inserts @p value (moved) at the given @p index, shifting subsequent elements to the right.
  /// @param[in] index Position at which to insert.
  /// @param[in,out] value Element to move-insert (left in a valid but unspecified state).
  /// @return Iterator to the inserted element.
  /// @pre index <= size()
  /// @pre size() < capacity()
  constexpr iterator insert(const size_type index, value_type&& value) noexcept
  {
    assert(index <= size());
    assert(size() < capacity());
    storage_.construct_for_overwrite_at(size());
    for (size_type i = size() - 1U; i > index; --i)
    {
      storage_[i] = std::move(storage_[i - 1U]);
    }
    storage_[index] = std::move(value);
    return begin() + static_cast<typename iterator::difference_type>(index);
  }

  /// @brief Inserts @p value before the element pointed to by @p pos.
  /// @param[in] pos Iterator before which the element will be inserted.
  /// @param[in] value Element to insert.
  /// @return Iterator to the inserted element.
  constexpr iterator insert(const_iterator pos, const value_type& value) noexcept
  {
    const auto index = static_cast<size_type>(pos - cbegin());
    return insert(index, value);
  }

  /// @brief Inserts @p value (moved) before the element pointed to by @p pos.
  /// @param[in] pos Iterator before which the element will be inserted.
  /// @param[in,out] value Element to move-insert (left in a valid but unspecified state).
  /// @return Iterator to the inserted element.
  constexpr iterator insert(const_iterator pos, value_type&& value) noexcept
  {
    const auto index = static_cast<size_type>(pos - cbegin());
    return insert(index, std::move(value));
  }

  /// @brief Erases the element at the given @p index, shifting subsequent elements to the left.
  /// @param[in] index Position of the element to erase.
  /// @return Iterator to the element following the erased one, or end() if the last element was erased.
  /// @pre index < size()
  constexpr iterator erase(const size_type index) noexcept
  {
    assert(index < size());
    for (size_type i = index; i < size() - 1U; ++i)
    {
      storage_[i] = std::move(storage_[i + 1U]);
    }
    storage_.destruct_at(size() - 1U);
    return begin() + static_cast<typename iterator::difference_type>(index);
  }

  /// @brief Erases the element pointed to by @p pos.
  /// @param[in] pos Iterator to the element to erase.
  /// @return Iterator to the element following the erased one, or end() if the last element was erased.
  constexpr iterator erase(const_iterator pos) noexcept
  {
    const auto index = static_cast<size_type>(pos - cbegin());
    return erase(index);
  }

  /// @brief Destroys all elements and sets size to zero.
  constexpr void clear() noexcept { storage_.clear(); }

  /// @brief Returns a reference to the element at @p index.
  /// @param[in] index Position of the element.
  /// @pre index < size()
  constexpr reference operator[](const size_type index) noexcept
  {
    assert(index < size());
    return storage_[index];
  }

  /// @brief Returns a const reference to the element at @p index.
  /// @param[in] index Position of the element.
  /// @pre index < size()
  constexpr const_reference operator[](const size_type index) const noexcept
  {
    assert(index < size());
    return storage_[index];
  }

  /// @brief Returns a reference to the first element.
  /// @pre !empty()
  constexpr reference front() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Returns a const reference to the first element.
  /// @pre !empty()
  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  /// @brief Returns a reference to the last element.
  /// @pre !empty()
  constexpr reference back() noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  /// @brief Returns a const reference to the last element.
  /// @pre !empty()
  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return storage_[size() - 1U];
  }

  /// @brief Returns a pointer to the underlying element storage.
  /// Valid even when the vector is empty (returns the base address of the storage).
  constexpr pointer data() noexcept { return storage_.data(); }

  /// @brief Returns a const pointer to the underlying element storage.
  /// Valid even when the vector is empty (returns the base address of the storage).
  constexpr const_pointer data() const noexcept { return storage_.data(); }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

  constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
  constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

  constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

private:
  StorageType storage_;
};

template <typename T>
using StaticVector = GenericStaticVector<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceStaticVector = GenericStaticVector<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
