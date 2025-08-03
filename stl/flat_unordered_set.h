#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename KeyT, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename KeyStorageT = ContiguousStorage<KeyT>>
class GenericFlatUnorderedSet
{
  using StorageType = KeyStorageT;

public:
  template <typename ValueRefT, typename ContainerT>
  class Iterator;

  using key_type = KeyT;
  using value_type = KeyT;
  using size_type = typename StorageType::size_type;
  using difference_type = std::ptrdiff_t;
  using reference = key_type&;
  using const_reference = const key_type&;
  using hasher = HashT;
  using key_equal = KeyEqualT;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename KT = key_type>
  constexpr bool emplace(KT&& key) noexcept
  {
    const auto hash_id = hasher_(key);
    for (size_type i = 0U; i < storage_.capacity(); ++i)
    {
      const size_type index = get_index_quadratic(hash_id, i);

      if (!storage_.is_constructed(index))
      {
        storage_.construct_at(index, std::forward<KT>(key));
        return true;
      }
    }

    // If we end up here, it means that the set is full and the key we tried to insert does not match any existing key.
    return false;
  }

  constexpr bool insert(const value_type& value) noexcept { return emplace(value); }
  constexpr bool insert(value_type&& value) noexcept { return emplace(std::move(value)); }

  constexpr bool erase(const key_type& key) noexcept
  {
    const auto it = find(key);
    if (it != end())
    {
      storage_.destruct_at(std::distance(begin(), it));
      return true;
    }

    return false;
  }

  constexpr void clear() noexcept { storage_.clear(); }

  constexpr iterator find(const key_type& key) noexcept { return find<iterator>(this, key); }
  constexpr const_iterator find(const key_type& key) const noexcept { return find<const_iterator>(this, key); }
  constexpr bool contains(const key_type& key) const noexcept { return find(key) != cend(); }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }
  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

protected:
  constexpr explicit GenericFlatUnorderedSet(const size_type capacity) noexcept : storage_{capacity} {}

private:
  constexpr size_type get_index_quadratic(const size_type hash_id, const size_type i) const noexcept
  {
    // Calculate the index using quadratic probing.
    return (hash_id + i * i) % storage_.capacity();
  }

  template <typename IteratorT, typename ContainerT>
  constexpr static IteratorT find(ContainerT* container, const key_type& key) noexcept
  {
    const auto hash_id = container->hasher_(key);
    for (size_type i = 0U; i < container->storage_.capacity(); ++i)
    {
      const size_type index = container->get_index_quadratic(hash_id, i);

      if (!container->storage_.is_constructed(index))
      {
        return container->end();
      }

      if (container->key_equal_(container->storage_[index], key))
      {
        return container->begin() + index;
      }
    }

    return container->end();
  }

  StorageType storage_;
  hasher hasher_{};
  key_equal key_equal_{};
};

template <typename KeyT, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>>
class FlatUnorderedSet : public GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, ContiguousStorage<KeyT>>
{
  using Base = GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, ContiguousStorage<KeyT>>;

public:
  using size_type = typename Base::size_type;

  explicit FlatUnorderedSet(const size_type capacity) noexcept : Base{capacity} {}
};

template <typename KeyT, std::size_t CAPACITY, typename HashT = std::hash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>>
class InplaceFlatUnorderedSet
    : public GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, InplaceContiguousStorage<KeyT, CAPACITY>>
{
  using Base = GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, InplaceContiguousStorage<KeyT, CAPACITY>>;

public:
  using size_type = typename Base::size_type;

  constexpr InplaceFlatUnorderedSet() noexcept : Base{CAPACITY} {}
};

} // namespace rtw::stl
