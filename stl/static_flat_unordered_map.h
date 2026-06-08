#pragma once

#include "stl/hash_iterator.h"
#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

/// @brief A fixed-capacity open-addressing hash map with quadratic probing.
///
/// GenericStaticFlatUnorderedMap stores key-value pairs in parallel flat contiguous buffers.
/// Uses quadratic probing for collision resolution and supports tombstone-aware
/// insertion (reuses slots from erased elements). If a key already exists, emplace
/// overwrites the mapped value. Never allocates dynamically when used with
/// InplaceStaticContiguousStorage.
///
/// @note Capacity should ideally be a prime number for optimal quadratic probing coverage.
/// With non-prime capacities, some slots may be unreachable, causing insertion to fail
/// even when the table is not full.
///
/// @note Iterator invalidation: erase() invalidates only the iterator to the erased element.
/// emplace()/insert()/operator[] do not invalidate existing iterators. clear() invalidates
/// all iterators.
///
/// @tparam KeyT Key type.
/// @tparam T Mapped value type.
/// @tparam HashT Hash function (default: std::hash<KeyT>).
/// @tparam KeyEqualT Key equality comparator (default: std::equal_to<KeyT>).
/// @tparam KeyStorageT Storage backend for keys.
/// @tparam ValueStorageT Storage backend for values.
///
/// Complexity:
///   - emplace / find / erase / operator[] / contains: O(1) average, O(n) worst case
///   - size / empty / capacity: O(1)
template <typename KeyT, typename T, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename KeyStorageT = StaticContiguousStorage<KeyT>, typename ValueStorageT = StaticContiguousStorage<T>>
class GenericStaticFlatUnorderedMap
{
  template <details::IteratorType, typename ValueRefT, typename ContainerT>
  friend class HashIterator;

public:
  using key_type = KeyT;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;
  using key_container_type = KeyStorageT;
  using value_container_type = ValueStorageT;
  using size_type = typename key_container_type::size_type;
  using difference_type = std::ptrdiff_t;
  using reference = std::pair<const key_type&, mapped_type&>;
  using const_reference = std::pair<const key_type&, const mapped_type&>;
  using hasher = HashT;
  using key_equal = KeyEqualT;
  using iterator = MapIterator<reference, GenericStaticFlatUnorderedMap>;
  using const_iterator = MapIterator<const_reference, const GenericStaticFlatUnorderedMap>;

  constexpr GenericStaticFlatUnorderedMap() noexcept = default;
  constexpr explicit GenericStaticFlatUnorderedMap(const size_type capacity) noexcept
      : keys_storage_{capacity}, values_storage_{capacity}
  {
  }

  constexpr size_type size() const noexcept { return keys_storage_.used_slots(); }
  constexpr bool empty() const noexcept { return keys_storage_.empty(); }
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return keys_storage_.capacity(); }

  /// @brief Inserts or overwrites a key-value pair.
  /// @param[in] key The key to insert or look up.
  /// @param[in] args Arguments forwarded to construct the mapped value.
  /// @return true if the key was inserted or updated, false only if the map is full and the key is absent.
  /// @note Tombstone (destructed) slots are reused for insertion. If the key already exists, the
  /// mapped value is overwritten.
  template <typename KT = key_type, typename... ArgsT>
  constexpr bool emplace(KT&& key, ArgsT&&... args) noexcept
  {
    const auto hash_id = hasher_(key);
    size_type first_available_index = keys_storage_.capacity();
    for (size_type i = 0U; i < keys_storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (keys_storage_.is_uninitialized(index))
      {
        const auto insert_index = (first_available_index < keys_storage_.capacity()) ? first_available_index : index;
        keys_storage_.construct_at(insert_index, std::forward<KT>(key));
        values_storage_.construct_at(insert_index, std::forward<ArgsT>(args)...);
        return true;
      }

      if (keys_storage_.is_destructed(index))
      {
        if (first_available_index == keys_storage_.capacity())
        {
          first_available_index = index;
        }
        continue;
      }

      if (key_equal_(keys_storage_[index], key))
      {
        values_storage_[index] = T{std::forward<ArgsT>(args)...};
        return true;
      }
    }

    if (first_available_index < keys_storage_.capacity())
    {
      keys_storage_.construct_at(first_available_index, std::forward<KT>(key));
      values_storage_.construct_at(first_available_index, std::forward<ArgsT>(args)...);
      return true;
    }

    // If we end up here, it means that the map is full and the key we tried to insert does not match any existing key.
    return false;
  }

  /// @brief Inserts a copy of @p value (pair).
  /// @param[in] value Key-value pair to insert.
  /// @return true if insertion took place or the value was updated.
  constexpr bool insert(const value_type& value) noexcept { return emplace(value.first, value.second); }

  /// @brief Inserts @p value (pair) by move.
  /// @param[in,out] value Key-value pair to move-insert.
  /// @return true if insertion took place or the value was updated.
  constexpr bool insert(value_type&& value) noexcept
  {
    auto&& [key, mapped_value] = std::move(value);
    return emplace(std::move(key), std::move(mapped_value));
  }

  /// @brief Erases the element with the given @p key.
  /// @param[in] key The key to erase.
  /// @return true if an element was erased.
  constexpr bool erase(const key_type& key) noexcept { return erase(find(key)); }

  /// @brief Erases the element pointed to by @p it.
  /// @param[in] it Iterator to the element to erase.
  /// @return true if an element was erased (i.e. it was not end()).
  constexpr bool erase(const iterator& it) noexcept
  {
    if (it != end())
    {
      keys_storage_.destruct_at(it.get_index());
      values_storage_.destruct_at(it.get_index());
      return true;
    }

    return false;
  }

  /// @brief Destroys all key-value pairs and resets all slots to uninitialized.
  constexpr void clear() noexcept
  {
    keys_storage_.clear();
    values_storage_.clear();
  }

  /// @brief Returns a reference to the mapped value for @p key, inserting a default if absent.
  /// @param[in] key The key to look up or insert.
  /// @return Reference to the mapped value.
  template <typename KT = key_type>
  constexpr mapped_type& operator[](KT&& key) noexcept
  {
    const auto hash_id = hasher_(key);
    size_type first_available_index = keys_storage_.capacity();
    for (size_type i = 0U; i < keys_storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (keys_storage_.is_uninitialized(index))
      {
        const auto insert_index = (first_available_index < keys_storage_.capacity()) ? first_available_index : index;
        keys_storage_.construct_at(insert_index, std::forward<KT>(key));
        return values_storage_.construct_for_overwrite_at(insert_index);
      }

      if (keys_storage_.is_destructed(index))
      {
        if (first_available_index == keys_storage_.capacity())
        {
          first_available_index = index;
        }
        continue;
      }

      if (key_equal_(keys_storage_[index], key))
      {
        return values_storage_[index];
      }
    }

    if (first_available_index < keys_storage_.capacity())
    {
      keys_storage_.construct_at(first_available_index, std::forward<KT>(key));
      return values_storage_.construct_for_overwrite_at(first_available_index);
    }

    assert(false && "StaticFlatUnorderedMap is full. Cannot insert new key.");
    __builtin_unreachable();
  }

  /// @brief Returns a const reference to the mapped value for @p key. Asserts if key is absent.
  /// @param[in] key The key to look up.
  /// @return Const reference to the mapped value.
  constexpr const mapped_type& operator[](const key_type& key) const noexcept
  {
    const auto it = find(key);
    assert(it != cend() && "Key not found in const operator[]");
    return it->second;
  }

  /// @brief Finds the element with the given @p key (mutable).
  /// @param[in] key The key to search for.
  /// @return Iterator to the element, or end() if not found.
  constexpr iterator find(const key_type& key) noexcept { return find<iterator>(this, key); }

  /// @brief Finds the element with the given @p key (const).
  /// @param[in] key The key to search for.
  /// @return Const iterator to the element, or cend() if not found.
  constexpr const_iterator find(const key_type& key) const noexcept { return find<const_iterator>(this, key); }

  /// @brief Checks whether the map contains the given @p key.
  /// @param[in] key The key to search for.
  /// @return true if the key is present.
  constexpr bool contains(const key_type& key) const noexcept { return find(key) != cend(); }

  constexpr const key_container_type& keys() const noexcept { return keys_storage_; }
  constexpr const value_container_type& values() const noexcept { return values_storage_; }

  constexpr iterator begin() noexcept { return iterator::make_begin_iterator(this); }
  constexpr const_iterator begin() const noexcept { return const_iterator::make_begin_iterator(this); }
  constexpr const_iterator cbegin() const noexcept { return const_iterator::make_begin_iterator(this); }
  constexpr iterator end() noexcept { return iterator::make_end_iterator(this); }
  constexpr const_iterator end() const noexcept { return const_iterator::make_end_iterator(this); }
  constexpr const_iterator cend() const noexcept { return const_iterator::make_end_iterator(this); }

private:
  constexpr size_type get_index_quadratic(const size_type hash_id, const size_type i) const noexcept
  {
    // Calculate the index using quadratic probing.
    return (hash_id + i * i) % keys_storage_.capacity();
  }

  template <typename IteratorT, typename ContainerT>
  constexpr static IteratorT find(ContainerT* container, const key_type& key) noexcept
  {
    const auto hash_id = container->hasher_(key);
    for (size_type i = 0U; i < container->keys_storage_.capacity(); ++i)
    {
      const size_type index = container->get_index_quadratic(hash_id, i);

      if (container->keys_storage_.is_uninitialized(index))
      {
        return container->end();
      }

      if (container->keys_storage_.is_destructed(index))
      {
        continue;
      }

      if (container->key_equal_(container->keys_storage_[index], key))
      {
        return IteratorT{container, index};
      }
    }

    return container->end();
  }

  key_container_type keys_storage_;
  value_container_type values_storage_;
  hasher hasher_{};
  key_equal key_equal_{};
};

template <typename KeyT, typename T, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>>
using StaticFlatUnorderedMap =
    GenericStaticFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, StaticContiguousStorage<KeyT>, StaticContiguousStorage<T>>;

template <typename KeyT, typename T, std::size_t CAPACITY, typename HashT = std::hash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>>
using InplaceStaticFlatUnorderedMap =
    GenericStaticFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, InplaceStaticContiguousStorage<KeyT, CAPACITY>,
                                  InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
