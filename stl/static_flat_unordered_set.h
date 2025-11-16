#pragma once

#include "stl/iterator.h"
#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

template <typename KeyT, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename KeyStorageT = StaticContiguousStorage<KeyT>>
class GenericStaticFlatUnorderedSet
{
  template <details::IteratorType, typename ValueRefT, typename ContainerT>
  friend class HashIterator;

public:
  using key_type = KeyT;
  using value_type = KeyT;
  using key_container_type = KeyStorageT;
  using size_type = typename key_container_type::size_type;
  using difference_type = std::ptrdiff_t;
  using reference = key_type&;
  using const_reference = const key_type&;
  using hasher = HashT;
  using key_equal = KeyEqualT;
  using const_iterator = SetIterator<const_reference, const GenericStaticFlatUnorderedSet>;

  constexpr GenericStaticFlatUnorderedSet() noexcept = default;
  constexpr explicit GenericStaticFlatUnorderedSet(const size_type capacity) noexcept : keys_storage_{capacity} {}

  constexpr size_type size() const noexcept { return keys_storage_.used_slots(); }
  constexpr bool empty() const noexcept { return keys_storage_.empty(); }
  constexpr size_type capacity() const noexcept { return keys_storage_.capacity(); }

  template <typename KT = key_type>
  constexpr bool emplace(KT&& key) noexcept
  {
    const auto hash_id = hasher_(key);
    for (size_type i = 0U; i < keys_storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (!keys_storage_.is_constructed(index))
      {
        keys_storage_.construct_at(index, std::forward<KT>(key));
        return true;
      }
    }

    // If we end up here, it means that the set is full and the key we tried to insert does not match any existing key.
    return false;
  }

  constexpr bool insert(const key_type& value) noexcept { return emplace(value); }
  constexpr bool insert(key_type&& value) noexcept { return emplace(std::move(value)); }

  constexpr bool erase(const key_type& key) noexcept
  {
    const auto it = find(key);
    if (it != end())
    {
      keys_storage_.destruct_at(it.get_index());
      return true;
    }

    return false;
  }

  constexpr bool erase(const const_iterator& it) noexcept
  {
    if (it != end())
    {
      keys_storage_.destruct_at(it.get_index());
      return false;
    }

    return true;
  }

  constexpr void clear() noexcept { keys_storage_.clear(); }

  constexpr const_iterator find(const key_type& key) const noexcept { return find<const_iterator>(this, key); }
  constexpr bool contains(const key_type& key) const noexcept { return find(key) != cend(); }

  constexpr const_iterator begin() const noexcept { return const_iterator::make_begin_iterator(this); }
  constexpr const_iterator cbegin() const noexcept { return const_iterator::make_begin_iterator(this); }
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

      if (!container->keys_storage_.is_constructed(index))
      {
        return container->end();
      }

      if (container->key_equal_(container->keys_storage_[index], key))
      {
        return IteratorT{container, index};
      }
    }

    return container->end();
  }

  key_container_type keys_storage_;
  hasher hasher_{};
  key_equal key_equal_{};
};

template <typename KeyT, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>>
using StaticFlatUnorderedSet = GenericStaticFlatUnorderedSet<KeyT, HashT, KeyEqualT, StaticContiguousStorage<KeyT>>;

template <typename KeyT, std::size_t CAPACITY, typename HashT = std::hash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>>
using InplaceStaticFlatUnorderedSet =
    GenericStaticFlatUnorderedSet<KeyT, HashT, KeyEqualT, InplaceStaticContiguousStorage<KeyT, CAPACITY>>;

} // namespace rtw::stl
