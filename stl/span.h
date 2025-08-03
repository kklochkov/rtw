#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>

namespace rtw::stl
{

template <typename T>
class Span
{
public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = pointer;
  using const_iterator = const_pointer;

  constexpr Span() noexcept = default;

  template <typename IteratorT, typename = std::enable_if_t<std::is_convertible_v<IteratorT, pointer>
                                                            || std::is_convertible_v<IteratorT, const_pointer>>>
  constexpr Span(IteratorT first, const size_type size) noexcept : data_{first}, size_{size}
  {
  }

  template <typename IteratorBeginT, typename IteratorEndT,
            typename = std::enable_if_t<(
                std::is_convertible_v<IteratorBeginT, pointer>
                || std::is_convertible_v<IteratorBeginT, const_pointer>)&&std::is_same_v<IteratorBeginT, IteratorEndT>>>
  constexpr Span(IteratorBeginT first, IteratorEndT last) noexcept
      : data_{first}, size_{static_cast<size_type>(last - first)}
  {
  }

  template <typename ContainerT,
            typename = std::enable_if_t<std::is_convertible_v<typename ContainerT::pointer, pointer>>>
  explicit Span(ContainerT& container) noexcept : data_{std::data(container)}, size_{std::size(container)}
  {
  }

  template <typename ContainerT,
            typename = std::enable_if_t<std::is_convertible_v<typename ContainerT::const_pointer, const_pointer>>>
  explicit Span(const ContainerT& container) noexcept : data_{std::data(container)}, size_{std::size(container)}
  {
  }

  template <size_type N>
  explicit Span(element_type (&array)[N]) noexcept : data_{array}, size_{N}
  {
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, pointer>>>
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions,-warnings-as-errors)
  constexpr Span(const Span<U>& other) noexcept : data_{other.data()}, size_{other.size()}
  {
  }

  constexpr size_type size() const noexcept { return size_; }
  constexpr size_type size_bytes() const noexcept { return size_ * sizeof(element_type); }
  constexpr bool empty() const noexcept { return size_ == 0U; }

  constexpr reference front() noexcept
  {
    assert(!empty());
    return data_[0];
  }
  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return data_[0];
  }

  constexpr reference back() noexcept
  {
    assert(!empty());
    return data_[size_ - 1];
  }

  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return data_[size_ - 1];
  }

  constexpr pointer data() noexcept { return data_; }
  constexpr const_pointer data() const noexcept { return data_; }

  constexpr reference operator[](const std::size_t index) noexcept
  {
    assert(!empty());
    return data_[index];
  }
  constexpr const_reference operator[](const std::size_t index) const noexcept
  {
    assert(!empty());
    return data_[index];
  }

  constexpr iterator begin() noexcept { return data_; }
  constexpr const_iterator begin() const noexcept { return data_; }
  constexpr const_iterator cbegin() const noexcept { return data_; }

  constexpr iterator end() noexcept
  {
    assert(!empty());
    return data_ + size_;
  }
  constexpr const_iterator end() const noexcept
  {
    assert(!empty());
    return data_ + size_;
  }
  constexpr const_iterator cend() const noexcept
  {
    assert(!empty());
    return data_ + size_;
  }

  constexpr Span<element_type> first(const size_type count) const noexcept
  {
    assert(count <= size_);
    return {data_, count};
  }

  constexpr Span<element_type> last(const size_type count) const noexcept
  {
    assert(count <= size_);
    return {data_ + size_ - count, count};
  }

  constexpr Span<element_type> subspan(const size_type offset,
                                       size_type count = std::numeric_limits<size_type>::max()) const noexcept
  {
    assert(offset <= size_);
    if (count == std::numeric_limits<size_type>::max() || offset + count > size_)
    {
      count = size_ - offset;
    }
    return {data_ + offset, count};
  }

private:
  pointer data_{nullptr};
  size_type size_{0U};
};

template <typename ContainerT, typename T = typename ContainerT::value_type>
constexpr Span<T> make_span(ContainerT& container) noexcept
{
  return Span<T>{container};
}

template <typename ContainerT, typename T = typename ContainerT::value_type>
constexpr Span<const T> make_span(const ContainerT& container) noexcept
{
  return Span<const T>{container};
}

template <typename T, std::size_t N>
constexpr Span<T> make_span(T (&array)[N]) noexcept
{
  return Span<T>{array};
}

template <typename T, std::size_t N>
constexpr Span<const T> make_span(const T (&array)[N]) noexcept
{
  return Span<const T>{array};
}

template <typename T>
constexpr Span<const std::byte> as_bytes(const Span<T>& span) noexcept
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return Span<const std::byte>{reinterpret_cast<const std::byte*>(span.data()), span.size_bytes()};
}

template <typename T>
constexpr Span<std::byte> as_writable_bytes(Span<T>& span) noexcept
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return Span<std::byte>{reinterpret_cast<std::byte*>(span.data()), span.size_bytes()};
}

} // namespace rtw::stl
