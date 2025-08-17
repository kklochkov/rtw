#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>

namespace rtw::stl
{

namespace details
{
struct ContiguousStorageIteratorTag : std::random_access_iterator_tag
{};

enum class IteratorType : std::uint8_t
{
  SET_ITERATOR,
  MAP_ITERATOR,
};

} // namespace details

template <typename ValueRefT, typename ContainerT>
class ContiguousStorageIterator
{
public:
  using value_type = typename ContainerT::value_type;
  using reference = ValueRefT;
  using pointer = std::add_pointer_t<reference>;
  using difference_type = typename ContainerT::difference_type;
  using iterator_category = details::ContiguousStorageIteratorTag;
  using iterator_concept = std::random_access_iterator_tag;
  using storage_type = typename ContainerT::storage_type;

  constexpr ContiguousStorageIterator(ContainerT* container, typename ContainerT::size_type index) noexcept
      : container_{container}, index_{index}
  {
  }

  constexpr bool is_constructed() const noexcept { return container_->is_constructed(index_); }

  constexpr reference operator*() const noexcept { return (*container_)[index_]; }
  constexpr pointer operator->() const noexcept { return &(*container_)[index_]; }

  constexpr storage_type& get_storage() noexcept { return container_->get_derived().get_storage(index_); }
  constexpr const storage_type& get_storage() const noexcept { return container_->get_derived().get_storage(index_); }

  constexpr ContiguousStorageIterator& operator++() noexcept
  {
    ++index_;
    return *this;
  }

  constexpr reference& operator[](const difference_type offset) const noexcept
  {
    return (*container_)[index_ + offset];
  }

  constexpr ContiguousStorageIterator operator++(int) noexcept
  {
    ContiguousStorageIterator temp{*this};
    ++(*this);
    return temp;
  }

  constexpr ContiguousStorageIterator& operator+=(const difference_type offset) noexcept
  {
    index_ += offset;
    return *this;
  }

  constexpr ContiguousStorageIterator& operator--() noexcept
  {
    --index_;
    return *this;
  }

  constexpr ContiguousStorageIterator operator--(int) noexcept
  {
    ContiguousStorageIterator temp{*this};
    --(*this);
    return temp;
  }

  constexpr ContiguousStorageIterator& operator-=(const difference_type offset) noexcept
  {
    index_ -= offset;
    return *this;
  }

  friend constexpr bool operator==(const ContiguousStorageIterator& lhs, const ContiguousStorageIterator& rhs) noexcept
  {
    return lhs.container_ == rhs.container_ && lhs.index_ == rhs.index_;
  }

  friend constexpr bool operator!=(const ContiguousStorageIterator& lhs, const ContiguousStorageIterator& rhs) noexcept
  {
    return !(lhs == rhs);
  }

  friend constexpr ContiguousStorageIterator operator+(const difference_type offset,
                                                       const ContiguousStorageIterator& it) noexcept
  {
    return ContiguousStorageIterator{it.container_, it.index_ + offset};
  }
  friend constexpr ContiguousStorageIterator operator+(const ContiguousStorageIterator& it,
                                                       const difference_type offset) noexcept
  {
    return ContiguousStorageIterator{it.container_, it.index_ + offset};
  }

  friend constexpr ContiguousStorageIterator operator-(const difference_type offset,
                                                       const ContiguousStorageIterator& it) noexcept
  {
    return ContiguousStorageIterator{it.container_, it.index_ - offset};
  }
  friend constexpr ContiguousStorageIterator operator-(const ContiguousStorageIterator& it,
                                                       const difference_type offset) noexcept
  {
    return ContiguousStorageIterator{it.container_, it.index_ - offset};
  }

  friend constexpr difference_type operator-(const ContiguousStorageIterator& lhs,
                                             const ContiguousStorageIterator& rhs) noexcept
  {
    assert(lhs.container_ == rhs.container_);
    return lhs.index_ - rhs.index_;
  }

private:
  ContainerT* container_{nullptr};
  typename ContainerT::size_type index_{0U};
};

template <details::IteratorType ITERATOR_TYPE, typename ValueRefT, typename ContainerT>
class HashIterator
{
  struct ReferenceWrapper
  {
    ValueRefT ref;

    const ValueRefT* operator->() const noexcept { return &ref; }
  };

public:
  using value_type = typename ContainerT::value_type;
  using reference = ValueRefT;
  using pointer = std::conditional_t<ITERATOR_TYPE == details::IteratorType::SET_ITERATOR,
                                     std::add_pointer_t<reference>, ReferenceWrapper>;
  using difference_type = typename ContainerT::difference_type;
  using size_type = typename ContainerT::size_type;
  using iterator_category = std::input_iterator_tag;
  using iterator_concept = std::random_access_iterator_tag;
  using container_type = ContainerT;

  constexpr HashIterator(ContainerT* container, const size_type index) noexcept : container_{container}, index_{index}
  {
    assert(container != nullptr);
  }

  constexpr static HashIterator make_begin_iterator(container_type* container) noexcept
  {
    // When the container is empty, use the capacity as the index to indicate the end.
    HashIterator it{container, container->empty() * container->capacity()};
    it.adjust_index();
    return it;
  }

  constexpr static HashIterator make_end_iterator(container_type* container) noexcept
  {
    return HashIterator{container, container->capacity()};
  }

  constexpr size_type get_index() const noexcept { return index_; }
  constexpr bool is_constructed() const noexcept { return container_->keys_storage_.is_constructed(index_); }

  constexpr reference operator*() const noexcept
  {
    if constexpr (ITERATOR_TYPE == details::IteratorType::SET_ITERATOR)
    {
      return container_->keys_storage_[index_];
    }
    else if constexpr (ITERATOR_TYPE == details::IteratorType::MAP_ITERATOR)
    {
      return {container_->keys_storage_[index_], container_->values_storage_[index_]};
    }
    else
    {
      static_assert(sizeof(ValueRefT) == 0U, "Unsupported iterator type."); // workaround before CWG2518/P2593R1
    }
  }
  constexpr pointer operator->() const noexcept
  {
    if constexpr (ITERATOR_TYPE == details::IteratorType::SET_ITERATOR)
    {
      return &operator*();
    }
    else if constexpr (ITERATOR_TYPE == details::IteratorType::MAP_ITERATOR)
    {
      return ReferenceWrapper{operator*()};
    }
    else
    {
      static_assert(sizeof(ValueRefT) == 0U, "Unsupported iterator type."); // workaround before CWG2518/P2593R1
    }
  }

  constexpr HashIterator& operator++() noexcept
  {
    ++index_;
    adjust_index();
    return *this;
  }

  constexpr HashIterator operator++(int) noexcept
  {
    HashIterator temp{*this};
    ++(*this);
    return temp;
  }

  friend constexpr bool operator==(const HashIterator& lhs, const HashIterator& rhs) noexcept
  {
    return (lhs.container_ == rhs.container_) && (lhs.index_ == rhs.index_);
  }

  friend constexpr bool operator!=(const HashIterator& lhs, const HashIterator& rhs) noexcept { return !(lhs == rhs); }

  friend constexpr difference_type operator-(const HashIterator& lhs, const HashIterator& rhs) noexcept
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
    for (; (index_ < container_->capacity()) && !container_->keys_storage_.is_constructed(index_); ++index_)
    {
    }
  }

  ContainerT* container_;
  size_type index_;
};

template <typename ValueRefT, typename ContainerT>
using SetIterator = HashIterator<details::IteratorType::SET_ITERATOR, ValueRefT, ContainerT>;

template <typename ValueRefT, typename ContainerT>
using MapIterator = HashIterator<details::IteratorType::MAP_ITERATOR, ValueRefT, ContainerT>;

} // namespace rtw::stl
