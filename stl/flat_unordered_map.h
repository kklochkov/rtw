#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename KeyT, typename T, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename KeyStorageT = ContiguousStorage<KeyT>, typename ValueStorageT = ContiguousStorage<T>>
class GenericFlatUnorderedMap
{
public:
  template <typename ValueRefT, typename ContainerT>
  class Iterator;

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
  using iterator = Iterator<reference, GenericFlatUnorderedMap>;
  using const_iterator = Iterator<const_reference, const GenericFlatUnorderedMap>;

  constexpr GenericFlatUnorderedMap() noexcept = default;

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

  constexpr bool erase(const key_type& key) noexcept
  {
    const auto it = find(key);
    if (it != end())
    {
      keys_storage_.destruct_at(it.get_index());
      values_storage_.destruct_at(it.get_index());
      return true;
    }

    return false;
  }

  constexpr bool erase(const iterator& it) noexcept
  {
    if (it != end())
    {
      keys_storage_.destruct_at(it.get_index());
      values_storage_.destruct_at(it.get_index());
      return false;
    }

    return true;
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

  constexpr iterator begin() noexcept { return iterator{this, 0U}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{this, 0U}; }
  constexpr const_iterator cbegin() const noexcept { return const_iterator{this, 0U}; }
  constexpr iterator end() noexcept { return iterator{this, keys_storage_.capacity()}; }
  constexpr const_iterator end() const noexcept { return const_iterator{this, keys_storage_.capacity()}; }
  constexpr const_iterator cend() const noexcept { return const_iterator{this, keys_storage_.capacity()}; }

protected:
  constexpr explicit GenericFlatUnorderedMap(const size_type capacity) noexcept
      : keys_storage_{capacity}, values_storage_{capacity}
  {
  }

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

template <typename KeyT, typename T, typename HashT, typename KeyEqualT, typename KeyStorageT, typename ValueStorageT>
template <typename ValueRefT, typename ContainerT>
class GenericFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, KeyStorageT, ValueStorageT>::Iterator
{
public:
  static_assert(
      std::is_same_v<ContainerT, GenericFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, KeyStorageT, ValueStorageT>>
          || std::is_same_v<ContainerT,
                            const GenericFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, KeyStorageT, ValueStorageT>>,
      "Iterator must be used with GenericFlatUnorderedMap or const GenericFlatUnorderedMap.");

  using reference = ValueRefT;
  using difference_type = typename ContainerT::difference_type;

  struct ReferenceWrapper
  {
    reference ref;

    const reference* operator->() const noexcept { return &ref; }
  };
  using pointer = ReferenceWrapper;

  constexpr Iterator(ContainerT* container, typename ContainerT::size_type index) noexcept
      : container_{container}, index_{index}
  {
  }

  constexpr size_t get_index() const noexcept { return index_; }

  constexpr reference operator*() const noexcept
  {
    return {container_->keys_storage_[index_], container_->values_storage_[index_]};
  }

  constexpr pointer operator->() const noexcept { return ReferenceWrapper{operator*()}; }

  constexpr Iterator& operator++() noexcept
  {
    ++index_;
    return *this;
  }

  constexpr Iterator operator++(int) noexcept
  {
    Iterator temp{*this};
    ++(*this);
    return temp;
  }

  constexpr Iterator operator+(const difference_type offset) const noexcept
  {
    return Iterator{container_, index_ + offset};
  }

  constexpr Iterator& operator+=(const difference_type offset) noexcept
  {
    index_ += offset;
    return *this;
  }

  constexpr Iterator& operator--() noexcept
  {
    --index_;
    return *this;
  }

  constexpr Iterator operator--(int) noexcept
  {
    Iterator temp{*this};
    --(*this);
    return temp;
  }

  constexpr Iterator operator-(const difference_type offset) const noexcept
  {
    return Iterator{container_, index_ - offset};
  }

  constexpr Iterator& operator-=(const difference_type offset) noexcept
  {
    index_ -= offset;
    return *this;
  }

  constexpr bool operator==(const Iterator& other) const noexcept { return index_ == other.index_; }
  constexpr bool operator!=(const Iterator& other) const noexcept { return !(*this == other); }

private:
  ContainerT* container_{nullptr};
  typename ContainerT::size_type index_{0U};
};

template <typename KeyT, typename T, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename BaseT =
              GenericFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, ContiguousStorage<KeyT>, ContiguousStorage<T>>>
class FlatUnorderedMap : public BaseT
{
public:
  explicit FlatUnorderedMap(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

template <typename KeyT, typename T, std::size_t CAPACITY, typename HashT = std::hash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>>
using InplaceFlatUnorderedMap =
    GenericFlatUnorderedMap<KeyT, T, HashT, KeyEqualT, InplaceContiguousStorage<KeyT, CAPACITY>,
                            InplaceContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
