#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

/// @brief A fixed-capacity circular (ring) buffer with random-access iterators.
///
/// GenericCircularBuffer supports push/pop at both front and back. When full, push_back
/// (emplace_back) overwrites the oldest (front) element, and push_front (emplace_front)
/// overwrites the newest (back) element. Logical indices map to physical positions via
/// modular arithmetic. Supports range-based for loops via random-access iterators.
///
/// @tparam T Element type.
/// @tparam StorageT Storage backend.
///
/// Complexity:
///   - push_front / push_back / emplace_front / emplace_back: O(1)
///   - pop_front / pop_back: O(1)
///   - operator[]: O(1)
///   - front / back: O(1)
///   - size / empty / capacity: O(1)
template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericCircularBuffer
{
  using StorageType = StorageT;

public:
  template <typename ValueRefT, typename ContainerT>
  class CircularBufferIterator;

  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;
  using difference_type = typename StorageType::difference_type;
  using iterator = CircularBufferIterator<reference, GenericCircularBuffer>;
  using const_iterator = CircularBufferIterator<const_reference, const GenericCircularBuffer>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  template <typename ValueRefT, typename ContainerT>
  class CircularBufferIterator
  {
  public:
    using value_type = typename ContainerT::value_type;
    using reference = ValueRefT;
    using pointer = std::add_pointer_t<reference>;
    using difference_type = typename ContainerT::difference_type;
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using storage_type = StorageType;

    constexpr CircularBufferIterator(ContainerT* container, const size_type index) noexcept
        : container_{container}, index_{index}
    {
    }

    constexpr reference operator*() const noexcept { return (*container_)[index_]; }
    constexpr pointer operator->() const noexcept { return &(*container_)[index_]; }

    constexpr CircularBufferIterator& operator++() noexcept
    {
      ++index_;
      return *this;
    }

    constexpr reference operator[](const difference_type offset) const noexcept
    {
      return (*container_)[index_ + offset];
    }

    constexpr CircularBufferIterator operator++(int) noexcept
    {
      CircularBufferIterator temp{*this};
      ++(*this);
      return temp;
    }

    constexpr CircularBufferIterator& operator+=(const difference_type offset) noexcept
    {
      index_ += offset;
      return *this;
    }

    constexpr CircularBufferIterator& operator--() noexcept
    {
      --index_;
      return *this;
    }

    constexpr CircularBufferIterator operator--(int) noexcept
    {
      CircularBufferIterator temp{*this};
      --(*this);
      return temp;
    }

    constexpr CircularBufferIterator& operator-=(const difference_type offset) noexcept
    {
      index_ -= offset;
      return *this;
    }

    friend constexpr bool operator==(const CircularBufferIterator& lhs, const CircularBufferIterator& rhs) noexcept
    {
      return (lhs.container_ == rhs.container_) && (lhs.index_ == rhs.index_);
    }

    friend constexpr bool operator!=(const CircularBufferIterator& lhs, const CircularBufferIterator& rhs) noexcept
    {
      return !(lhs == rhs);
    }

    friend constexpr bool operator<(const CircularBufferIterator& lhs, const CircularBufferIterator& rhs) noexcept
    {
      return (lhs.container_ == rhs.container_) && (lhs.index_ < rhs.index_);
    }

    friend constexpr bool operator>(const CircularBufferIterator& lhs, const CircularBufferIterator& rhs) noexcept
    {
      return (lhs.container_ == rhs.container_) && (lhs.index_ > rhs.index_);
    }

    friend constexpr CircularBufferIterator operator+(const difference_type offset,
                                                      const CircularBufferIterator& it) noexcept
    {
      return CircularBufferIterator{it.container_, it.index_ + offset};
    }
    friend constexpr CircularBufferIterator operator+(const CircularBufferIterator& it,
                                                      const difference_type offset) noexcept
    {
      return CircularBufferIterator{it.container_, it.index_ + offset};
    }

    friend constexpr CircularBufferIterator operator-(const CircularBufferIterator& it,
                                                      const difference_type offset) noexcept
    {
      return CircularBufferIterator{it.container_, it.index_ - offset};
    }

    friend constexpr difference_type operator-(const CircularBufferIterator& lhs,
                                               const CircularBufferIterator& rhs) noexcept
    {
      assert(lhs.container_ == rhs.container_);
      return lhs.index_ - rhs.index_;
    }

  private:
    ContainerT* container_{nullptr};
    size_type index_{0U};
  };

  constexpr GenericCircularBuffer() noexcept = default;
  constexpr explicit GenericCircularBuffer(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr bool full() const noexcept { return size() == capacity(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  /// @brief Constructs an element in-place at the front of the buffer. Overwrites the newest (back) element if full.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the newly constructed element.
  template <typename... ArgsT>
  constexpr reference emplace_front(ArgsT&&... args) noexcept
  {
    head_ = (head_ + capacity() - 1U) % capacity();
    return emplace_at_index(head_, std::forward<ArgsT>(args)...);
  }

  /// @brief Pushes @p value to the front of the buffer.
  /// @param[in] value Element to push.
  template <typename U = T>
  constexpr void push_front(U&& value) noexcept
  {
    emplace_front(std::forward<U>(value));
  }

  /// @brief Removes the front element.
  /// @pre !empty()
  constexpr void pop_front() noexcept
  {
    assert(!empty());
    storage_.destruct_at(head_);
    advance_head();
  }

  /// @brief Constructs an element in-place at the back. Overwrites the oldest element if full.
  /// @param[in] args Arguments forwarded to the element constructor.
  /// @return Reference to the newly constructed element.
  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    const auto index = (head_ + size()) % capacity();

    if (size() == capacity())
    {
      // Start overwrite old data.
      advance_head();
    }

    return emplace_at_index(index, std::forward<ArgsT>(args)...);
  }

  /// @brief Pushes @p value to the back. Overwrites the oldest element if full.
  /// @param[in] value Element to push.
  template <typename U = T>
  constexpr void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  /// @brief Removes the back element.
  /// @pre !empty()
  constexpr void pop_back() noexcept
  {
    assert(!empty());
    storage_.destruct_at(get_back_index());
  }

  /// @brief Destroys all elements and resets the head index.
  constexpr void clear() noexcept
  {
    head_ = 0U;
    storage_.clear();
  }

  /// @brief Returns a reference to the front (oldest) element.
  /// @pre !empty()
  constexpr reference front() noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  /// @brief Returns a const reference to the front (oldest) element.
  /// @pre !empty()
  constexpr const_reference front() const noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  /// @brief Returns a reference to the back (newest) element.
  /// @pre !empty()
  constexpr reference back() noexcept
  {
    assert(!empty());
    return storage_[get_back_index()];
  }

  /// @brief Returns a const reference to the back (newest) element.
  /// @pre !empty()
  constexpr const_reference back() const noexcept
  {
    assert(!empty());
    return storage_[get_back_index()];
  }

  /// @brief Returns a reference to the element at logical @p index (0 = front).
  /// @param[in] index Logical position relative to front.
  /// @pre index < size()
  constexpr reference operator[](const size_type index) noexcept
  {
    assert(index < size());
    return storage_[get_index(index)];
  }

  /// @brief Returns a const reference to the element at logical @p index (0 = front).
  /// @param[in] index Logical position relative to front.
  /// @pre index < size()
  constexpr const_reference operator[](const size_type index) const noexcept
  {
    assert(index < size());
    return storage_[get_index(index)];
  }

  constexpr iterator begin() noexcept { return iterator{this, 0U}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{this, 0U}; }
  constexpr const_iterator cbegin() const noexcept { return const_iterator{this, 0U}; }

  constexpr iterator end() noexcept { return iterator{this, size()}; }
  constexpr const_iterator end() const noexcept { return const_iterator{this, size()}; }
  constexpr const_iterator cend() const noexcept { return const_iterator{this, size()}; }

  constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
  constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

  constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

private:
  constexpr void advance_head() noexcept { head_ = (head_ + 1U) % capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_at_index(const size_type index, ArgsT&&... args) noexcept
  {
    if (storage_.is_constructed(index))
    {
      storage_.destruct_at(index);
    }
    return storage_.construct_at(index, std::forward<ArgsT>(args)...);
  }

  constexpr size_type get_index(const size_type index) const noexcept { return (head_ + index) % capacity(); }
  constexpr size_type get_back_index() const noexcept { return (head_ + size() - 1U) % capacity(); }

  StorageType storage_;
  size_type head_{0U};
};

template <typename T>
using CircularBuffer = GenericCircularBuffer<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceCircularBuffer = GenericCircularBuffer<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
