#pragma once

#include <array>
#include <cassert>
#include <memory>

namespace rtw::stl
{

namespace details
{
template <typename T>
constexpr inline bool IS_TRIVIAL_V =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
} // namespace details

template <typename T, bool IS_TRIVIAL>
class AlignedObjectStorage;

template <typename T>
class AlignedObjectStorage<T, true>
{
public:
  using is_trivial = std::true_type;
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  constexpr AlignedObjectStorage() noexcept : constructed_{false} {}

  constexpr bool is_constructed() const noexcept { return constructed_; }

  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T{std::forward<ArgsT>(args)...};
    constructed_ = true;
    return *value;
  }

  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T;
    constructed_ = true;
    return *value;
  }

  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // For trivially destructible types explicitly calling destruct is not necessary.
      constructed_ = false;
    }
  }

  constexpr pointer get_pointer() noexcept
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::launder(reinterpret_cast<T*>(data_.data()));
  }
  constexpr const_pointer get_pointer() const noexcept
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::launder(reinterpret_cast<const T*>(data_.data()));
  }
  constexpr pointer operator->() noexcept { return get_pointer(); }
  constexpr const_pointer operator->() const noexcept { return get_pointer(); }

  constexpr reference get_reference() noexcept { return *get_pointer(); }
  constexpr const_reference get_reference() const noexcept { return *get_pointer(); }
  constexpr reference operator*() noexcept { return get_reference(); }
  constexpr const_reference operator*() const noexcept { return get_reference(); }

  constexpr const std::byte* get_raw_pointer() const noexcept { return data_.data(); }
  constexpr std::size_t get_raw_size() const noexcept { return data_.size(); }

private:
  alignas(alignof(T)) std::array<std::byte, sizeof(T)> data_{};
  bool constructed_ : 1;
};

template <typename T>
class AlignedObjectStorage<T, false>
{
public:
  using is_trivial = std::false_type;
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  constexpr AlignedObjectStorage() noexcept : constructed_{false} {}

  constexpr bool is_constructed() const noexcept { return constructed_; }

  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    data_ = T{std::forward<ArgsT>(args)...};
    constructed_ = true;
    return data_;
  }

  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    data_ = T{};
    constructed_ = true;
    return data_;
  }

  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // No need to explicitly call the destructor, bease it will be called either during new value assignment or when
      // during desturction of the storage.
      constructed_ = false;
    }
  }

  constexpr pointer get_pointer() noexcept { return &data_; }
  constexpr const_pointer get_pointer() const noexcept { return &data_; }
  constexpr pointer operator->() noexcept { return get_pointer(); }
  constexpr const_pointer operator->() const noexcept { return get_pointer(); }

  constexpr reference get_reference() noexcept { return *get_pointer(); }
  constexpr const_reference get_reference() const noexcept { return *get_pointer(); }
  constexpr reference operator*() noexcept { return get_reference(); }
  constexpr const_reference operator*() const noexcept { return get_reference(); }

  constexpr const std::byte* get_raw_pointer() const noexcept { return nullptr; }
  constexpr std::size_t get_raw_size() const noexcept { return 0U; }

private:
  T data_;
  bool constructed_ : 1;
};

template <typename T, typename DerivedT>
class GenericContiguousStorage
{
public:
  template <typename ValueRefT, typename ContainerT>
  class Iterator;

  using storage_type = AlignedObjectStorage<T, details::IS_TRIVIAL_V<T>>;
  using value_type = typename storage_type::value_type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename storage_type::reference;
  using const_reference = typename storage_type::const_reference;
  using pointer = typename storage_type::pointer;
  using const_pointer = typename storage_type::const_pointer;
  using iterator = Iterator<reference, GenericContiguousStorage>;
  using const_iterator = Iterator<const_reference, const GenericContiguousStorage>;

  constexpr size_type used_slots() const noexcept { return used_slots_; }
  constexpr bool empty() const noexcept { return used_slots_ == 0U; }
  constexpr size_type capacity() const noexcept { return capacity_; }

  template <typename... ArgsT>
  constexpr reference construct_at(const size_type index, ArgsT&&... args) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct(std::forward<ArgsT>(args)...);
    ++used_slots_;
    return value;
  }

  constexpr reference construct_for_overwrite_at(const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct_for_overwrite_at();
    ++used_slots_;
    return value;
  }

  constexpr void destruct_at(const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    storage.destruct();
    --used_slots_;
  }

  constexpr bool is_constructed(const size_type index) const noexcept
  {
    const auto& storage = get_derived().get_storage(index);
    return storage.is_constructed();
  }

  constexpr reference operator[](const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    return storage.get_reference();
  }

  constexpr const_reference operator[](const size_type index) const noexcept
  {
    const auto& storage = get_derived().get_storage(index);
    return storage.get_reference();
  }

  constexpr void clear() noexcept
  {
    for (size_type index = 0U; index < capacity_; ++index)
    {
      auto& storage = get_derived().get_storage(index);
      storage.destruct();
    }
    used_slots_ = 0U;
  }

  constexpr iterator begin() noexcept { return iterator{this, 0U}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{this, 0U}; }
  constexpr const_iterator cbegin() const noexcept { return const_iterator{this, 0U}; }

  constexpr iterator end() noexcept { return iterator{this, used_slots_}; }
  constexpr const_iterator end() const noexcept { return const_iterator{this, used_slots_}; }
  constexpr const_iterator cend() const noexcept { return const_iterator{this, used_slots_}; }

protected:
  constexpr explicit GenericContiguousStorage(const size_t capacity) noexcept : capacity_{capacity}
  {
    assert(capacity > 0U);
  }

private:
  constexpr DerivedT& get_derived() noexcept { return static_cast<DerivedT&>(*this); }
  constexpr const DerivedT& get_derived() const noexcept { return static_cast<const DerivedT&>(*this); }

  size_type used_slots_{0U};
  size_type capacity_{0U};
};

namespace details
{

struct ContiguousStorageIteratorTag : std::input_iterator_tag
{};

}; // namespace details

template <typename T, typename DerivedT>
template <typename ValueRefT, typename ContainerT>
class GenericContiguousStorage<T, DerivedT>::Iterator
{
public:
  static_assert(std::is_same_v<ContainerT, GenericContiguousStorage<T, DerivedT>>
                    || std::is_same_v<ContainerT, const GenericContiguousStorage<T, DerivedT>>,
                "Iterator must be used with GenericContiguousStorage or const GenericContiguousStorage.");

  using value_type = typename ContainerT::value_type;
  using reference = ValueRefT;
  using pointer = std::add_pointer_t<reference>;
  using difference_type = typename ContainerT::difference_type;
  using iterator_category = details::ContiguousStorageIteratorTag;
  using iterator_concept = std::random_access_iterator_tag;
  using storage_type = typename ContainerT::storage_type;

  constexpr Iterator(ContainerT* container, typename ContainerT::size_type index) noexcept
      : container_{container}, index_{index}
  {
  }

  constexpr size_t get_index() const noexcept { return index_; }
  constexpr bool is_constructed() const noexcept { return container_->is_constructed(index_); }

  constexpr reference operator*() const noexcept { return (*container_)[index_]; }
  constexpr pointer operator->() const noexcept { return &(*container_)[index_]; }

  constexpr storage_type& get_storage() noexcept { return container_->get_derived().get_storage(index_); }
  constexpr const storage_type& get_storage() const noexcept { return container_->get_derived().get_storage(index_); }

  constexpr Iterator& operator++() noexcept
  {
    ++index_;
    return *this;
  }

  constexpr reference& operator[](const difference_type offset) const noexcept
  {
    return (*container_)[index_ + offset];
  }

  constexpr Iterator operator++(int) noexcept
  {
    Iterator temp{*this};
    ++(*this);
    return temp;
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

  constexpr Iterator& operator-=(const difference_type offset) noexcept
  {
    index_ -= offset;
    return *this;
  }

  friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept
  {
    return lhs.container_ == rhs.container_ && lhs.index_ == rhs.index_;
  }

  friend constexpr bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept { return !(lhs == rhs); }

  friend constexpr Iterator operator+(const difference_type offset, const Iterator& it) noexcept
  {
    return Iterator{it.container_, it.index_ + offset};
  }
  friend constexpr Iterator operator+(const Iterator& it, const difference_type offset) noexcept
  {
    return Iterator{it.container_, it.index_ + offset};
  }

  friend constexpr Iterator operator-(const difference_type offset, const Iterator& it) noexcept
  {
    return Iterator{it.container_, it.index_ - offset};
  }
  friend constexpr Iterator operator-(const Iterator& it, const difference_type offset) noexcept
  {
    return Iterator{it.container_, it.index_ - offset};
  }

  friend constexpr difference_type operator-(const Iterator& lhs, const Iterator& rhs) noexcept
  {
    assert(lhs.container_ == rhs.container_);
    return lhs.index_ - rhs.index_;
  }

private:
  ContainerT* container_{nullptr};
  typename ContainerT::size_type index_{0U};
};

template <typename T>
class ContiguousStorage : public GenericContiguousStorage<T, ContiguousStorage<T>>
{
  using Base = GenericContiguousStorage<T, ContiguousStorage<T>>;

  friend Base;

public:
  using storage_type = typename Base::storage_type;
  using size_type = typename Base::size_type;

  explicit ContiguousStorage(const size_type capacity) noexcept
      : Base{capacity}, data_{std::make_unique<storage_type[]>(capacity)}
  {
  }

private:
  storage_type& get_storage(const size_type index) noexcept
  {
    assert(index < Base::capacity());
    return data_.get()[index];
  }

  const storage_type& get_storage(const size_type index) const noexcept
  {
    assert(index < Base::capacity());
    return data_.get()[index];
  }

  storage_type* get_storage() noexcept { return data_.get(); }
  const storage_type* get_storage() const noexcept { return data_.get(); }

  std::unique_ptr<storage_type[]> data_;
};

template <typename T, std::size_t CAPACITY>
class InplaceContiguousStorage : public GenericContiguousStorage<T, InplaceContiguousStorage<T, CAPACITY>>
{
  using Base = GenericContiguousStorage<T, InplaceContiguousStorage<T, CAPACITY>>;

  friend Base;

public:
  static_assert(CAPACITY > 0U, "CAPACITY must be greater than 0.");

  using storage_type = typename Base::storage_type;
  using size_type = typename Base::size_type;

  constexpr InplaceContiguousStorage() noexcept : Base{CAPACITY} {}

private:
  constexpr storage_type& get_storage(const size_type index) noexcept
  {
    assert(index < Base::capacity());
    return data_[index];
  }

  constexpr const storage_type& get_storage(const size_type index) const noexcept
  {
    assert(index < Base::capacity());
    return data_[index];
  }

  constexpr storage_type* get_storage() noexcept { return data_.data(); }
  constexpr const storage_type* get_storage() const noexcept { return data_.data(); }

  alignas(storage_type) std::array<storage_type, CAPACITY> data_{};
};

template <typename IteratorT>
constexpr bool is_memory_contiguous(IteratorT begin, IteratorT end) noexcept
{
  bool contiguous = true;

  const auto size = std::distance(begin, end);
  for (std::ptrdiff_t i = 0U; i < size; ++i)
  {
    if constexpr (std::is_same_v<typename IteratorT::iterator_category, details::ContiguousStorageIteratorTag>)
    {
      // The special threatment is required, because of the AlignedObjectStorage,
      // since this class manages wrapped object memory and its lifetime.
      // So, we would like to check if all AlignedObjectStorage are laid out in the same contiguous memory.
      const auto* a = &*(std::next(begin.get_storage().get_pointer(), i));
      const auto* b = &*(std::next(&*begin.get_storage(), i));
      contiguous &= a == b;
    }
    else
    {
      const auto* a = &*(std::next(begin, i));
      const auto* b = &*(std::next(&*begin, i));
      contiguous &= a == b;
    }
  }

  return contiguous;
}

} // namespace rtw::stl
