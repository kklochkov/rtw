#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>

namespace rtw::stl
{

namespace details
{

/// @brief Custom iterator tag that extends std::random_access_iterator_tag.
/// Used to identify StaticContiguousStorage iterators for memory-contiguity checks.
struct StaticContiguousStorageIteratorTag : std::random_access_iterator_tag
{};

} // namespace details

/// @brief Random-access iterator for GenericStaticContiguousStorage and its derived containers.
///
/// Stores a pointer to the container and a logical index. Provides full random-access semantics
/// (++, --, +=, -=, +, -, [], <, >, ==, !=). Dereferences through the container's operator[].
///
/// @tparam ValueRefT Reference type (reference or const_reference).
/// @tparam ContainerT The container type being iterated.
template <typename ValueRefT, typename ContainerT>
class StaticContiguousStorageIterator
{
public:
  using value_type = typename ContainerT::value_type;
  using reference = ValueRefT;
  using pointer = std::add_pointer_t<reference>;
  using difference_type = typename ContainerT::difference_type;
  using iterator_category = details::StaticContiguousStorageIteratorTag;
  using iterator_concept = std::random_access_iterator_tag;
  using storage_type = typename ContainerT::storage_type;

  constexpr StaticContiguousStorageIterator(ContainerT* container, typename ContainerT::size_type index) noexcept
      : container_{container}, index_{index}
  {
  }

  constexpr bool is_constructed() const noexcept { return container_->is_constructed(index_); }

  constexpr reference operator*() const noexcept { return (*container_)[index_]; }
  constexpr pointer operator->() const noexcept { return &(*container_)[index_]; }

  constexpr storage_type& get_storage() noexcept { return container_->get_derived().get_storage(index_); }
  constexpr const storage_type& get_storage() const noexcept { return container_->get_derived().get_storage(index_); }

  constexpr StaticContiguousStorageIterator& operator++() noexcept
  {
    ++index_;
    return *this;
  }

  constexpr reference operator[](const difference_type offset) const noexcept { return (*container_)[index_ + offset]; }

  constexpr StaticContiguousStorageIterator operator++(int) noexcept
  {
    StaticContiguousStorageIterator temp{*this};
    ++(*this);
    return temp;
  }

  constexpr StaticContiguousStorageIterator& operator+=(const difference_type offset) noexcept
  {
    index_ += offset;
    return *this;
  }

  constexpr StaticContiguousStorageIterator& operator--() noexcept
  {
    --index_;
    return *this;
  }

  constexpr StaticContiguousStorageIterator operator--(int) noexcept
  {
    StaticContiguousStorageIterator temp{*this};
    --(*this);
    return temp;
  }

  constexpr StaticContiguousStorageIterator& operator-=(const difference_type offset) noexcept
  {
    index_ -= offset;
    return *this;
  }

  friend constexpr bool operator==(const StaticContiguousStorageIterator& lhs,
                                   const StaticContiguousStorageIterator& rhs) noexcept
  {
    return (lhs.container_ == rhs.container_) && (lhs.index_ == rhs.index_);
  }

  friend constexpr bool operator!=(const StaticContiguousStorageIterator& lhs,
                                   const StaticContiguousStorageIterator& rhs) noexcept
  {
    return !(lhs == rhs);
  }

  friend constexpr bool operator<(const StaticContiguousStorageIterator& lhs,
                                  const StaticContiguousStorageIterator& rhs) noexcept
  {
    return (lhs.container_ == rhs.container_) && (lhs.index_ < rhs.index_);
  }

  friend constexpr bool operator>(const StaticContiguousStorageIterator& lhs,
                                  const StaticContiguousStorageIterator& rhs) noexcept
  {
    return (lhs.container_ == rhs.container_) && (lhs.index_ > rhs.index_);
  }

  friend constexpr StaticContiguousStorageIterator operator+(const difference_type offset,
                                                             const StaticContiguousStorageIterator& it) noexcept
  {
    return StaticContiguousStorageIterator{it.container_, it.index_ + offset};
  }
  friend constexpr StaticContiguousStorageIterator operator+(const StaticContiguousStorageIterator& it,
                                                             const difference_type offset) noexcept
  {
    return StaticContiguousStorageIterator{it.container_, it.index_ + offset};
  }

  friend constexpr StaticContiguousStorageIterator operator-(const StaticContiguousStorageIterator& it,
                                                             const difference_type offset) noexcept
  {
    return StaticContiguousStorageIterator{it.container_, it.index_ - offset};
  }

  friend constexpr difference_type operator-(const StaticContiguousStorageIterator& lhs,
                                             const StaticContiguousStorageIterator& rhs) noexcept
  {
    assert(lhs.container_ == rhs.container_);
    return lhs.index_ - rhs.index_;
  }

private:
  ContainerT* container_{nullptr};
  typename ContainerT::size_type index_{0U};
};

} // namespace rtw::stl
