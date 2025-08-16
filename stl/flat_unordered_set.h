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
  using iterator = Iterator<reference, GenericFlatUnorderedSet>;
  using const_iterator = Iterator<const_reference, const GenericFlatUnorderedSet>;

  constexpr GenericFlatUnorderedSet() noexcept = default;

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
      storage_.destruct_at(it.get_index());
      return true;
    }

    return false;
  }

  constexpr void clear() noexcept { storage_.clear(); }

  constexpr iterator find(const key_type& key) noexcept { return find<iterator>(this, key); }
  constexpr const_iterator find(const key_type& key) const noexcept { return find<const_iterator>(this, key); }
  constexpr bool contains(const key_type& key) const noexcept { return find(key) != cend(); }

  constexpr iterator begin() noexcept { return iterator::make_begin_iterator(this); }
  constexpr const_iterator begin() const noexcept { return const_iterator::make_begin_iterator(this); }
  constexpr const_iterator cbegin() const noexcept { return const_iterator::make_begin_iterator(this); }
  constexpr iterator end() noexcept { return iterator::make_end_iterator(this); }
  constexpr const_iterator end() const noexcept { return const_iterator::make_end_iterator(this); }
  constexpr const_iterator cend() const noexcept { return const_iterator::make_end_iterator(this); }

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
        return IteratorT{container, index};
      }
    }

    return container->end();
  }

  StorageType storage_;
  hasher hasher_{};
  key_equal key_equal_{};
};

template <typename KeyT, typename HashT, typename KeyEqualT, typename KeyStorageT>
template <typename ValueRefT, typename ContainerT>
class GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, KeyStorageT>::Iterator
{
public:
  static_assert(std::is_same_v<ContainerT, GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, KeyStorageT>>
                    || std::is_same_v<ContainerT, const GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, KeyStorageT>>,
                "Iterator must be used with GenericFlatUnorderedSet or const GenericFlatUnorderedSet.");

  using value_type = typename ContainerT::value_type;
  using reference = ValueRefT;
  using pointer = std::add_pointer_t<reference>;
  using difference_type = typename ContainerT::difference_type;
  using size_type = typename ContainerT::size_type;
  using iterator_category = std::input_iterator_tag;
  using iterator_concept = std::random_access_iterator_tag;

  constexpr Iterator(ContainerT* container, const size_type index) noexcept : container_{container}, index_{index}
  {
    assert(container != nullptr);
  }

  constexpr static Iterator make_begin_iterator(ContainerT* container) noexcept
  {
    // When the container is empty, use the capacity as the index to indicate the end.
    Iterator it{container, container->empty() * container->capacity()};
    it.adjust_index();
    return it;
  }

  constexpr static Iterator make_end_iterator(ContainerT* container) noexcept
  {
    return Iterator{container, container->capacity()};
  }

  constexpr size_type get_index() const noexcept { return index_; }
  constexpr bool is_constructed() const noexcept { return container_->storage_.is_constructed(index_); }

  constexpr reference operator*() const noexcept { return container_->storage_[index_]; }
  constexpr pointer operator->() const noexcept { return *operator*(); }

  constexpr Iterator& operator++() noexcept
  {
    ++index_;
    adjust_index();
    return *this;
  }

  constexpr Iterator operator++(int) noexcept
  {
    Iterator temp{*this};
    ++(*this);
    return temp;
  }

  friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept
  {
    return (lhs.container_ == rhs.container_) && (lhs.index_ == rhs.index_);
  }

  friend constexpr bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept { return !(lhs == rhs); }

  friend constexpr difference_type operator-(const Iterator& lhs, const Iterator& rhs) noexcept
  {
    assert(lhs.container_ == rhs.container_);
    if (((lhs.index_ < lhs.container_->capacity()) && (rhs.index_ < rhs.container_->capacity()))
        || ((lhs.index_ >= lhs.container_->capacity()) && (rhs.index_ >= rhs.container_->capacity())))
    {
      return static_cast<difference_type>(lhs.index_) - static_cast<difference_type>(rhs.index_);
    }

    if (lhs.index_ >= lhs.container_->capacity())
    {
      return static_cast<difference_type>(lhs.container_->size()) - static_cast<difference_type>(rhs.index_);
    }

    return static_cast<difference_type>(lhs.index_) - static_cast<difference_type>(rhs.container_->size());
  }

private:
  constexpr void adjust_index() noexcept
  {
    // Adjust the index to point to the first constructed element or the end of the container.
    for (; (index_ < container_->capacity()) && !container_->storage_.is_constructed(index_); ++index_)
    {
    }
  }

  ContainerT* container_;
  size_type index_;
};

template <typename KeyT, typename HashT = std::hash<KeyT>, typename KeyEqualT = std::equal_to<KeyT>,
          typename BaseT = GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, ContiguousStorage<KeyT>>>
class FlatUnorderedSet : public BaseT
{
public:
  explicit FlatUnorderedSet(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

template <typename KeyT, std::size_t CAPACITY, typename HashT = std::hash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>>
using InplaceFlatUnorderedSet =
    GenericFlatUnorderedSet<KeyT, HashT, KeyEqualT, InplaceContiguousStorage<KeyT, CAPACITY>>;

} // namespace rtw::stl
