#pragma once

#include "stl/iterator.h"
#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

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
  constexpr size_type capacity() const noexcept { return keys_storage_.capacity(); }

  template <typename KT = key_type, typename... ArgsT>
  constexpr bool emplace(KT&& key, ArgsT&&... args) noexcept
  {
    const auto hash_id = hasher_(key);
    for (size_type i = 0U; i < keys_storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (!keys_storage_.is_constructed(index))
      {
        keys_storage_.construct_at(index, std::forward<KT>(key));
        values_storage_.construct_at(index, std::forward<ArgsT>(args)...);
        return true;
      }

      if (key_equal_(keys_storage_[index], key))
      {
        values_storage_[index] = T{std::forward<ArgsT>(args)...};
        return true;
      }
    }

    // If we end up here, it means that the map is full and the key we tried to insert does not match any existing key.
    return false;
  }

  constexpr bool insert(const value_type& value) noexcept { return emplace(value.first, value.second); }
  constexpr bool insert(value_type&& value) noexcept
  {
    auto&& [key, mapped_value] = std::move(value);
    return emplace(std::move(key), std::move(mapped_value));
  }

  constexpr bool erase(const key_type& key) noexcept { return erase(find(key)); }
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

  constexpr void clear() noexcept
  {
    keys_storage_.clear();
    values_storage_.clear();
  }

  template <typename KT = key_type>
  constexpr mapped_type& operator[](KT&& key) noexcept
  {
    const auto hash_id = hasher_(key);
    for (size_type i = 0U; i < keys_storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (!keys_storage_.is_constructed(index))
      {
        keys_storage_.construct_at(index, std::forward<KT>(key));
        return values_storage_.construct_for_overwrite_at(index);
      }

      if (key_equal_(keys_storage_[index], key))
      {
        return values_storage_[index];
      }
    }

    // This is treated by the Storage.
    return end()->second;
  }

  constexpr const mapped_type& operator[](const key_type& key) const noexcept { return find(key)->second; }

  constexpr iterator find(const key_type& key) noexcept { return find<iterator>(this, key); }
  constexpr const_iterator find(const key_type& key) const noexcept { return find<const_iterator>(this, key); }
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
