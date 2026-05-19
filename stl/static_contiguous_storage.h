#pragma once

#include "stl/iterator.h"

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

enum class ObjectStorageState : std::uint8_t
{
  UNINITIALIZED = 0U,
  CONSTRUCTED,
  DESTRUCTED
};

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

  constexpr AlignedObjectStorage() noexcept : state_{details::ObjectStorageState::UNINITIALIZED} {}

  constexpr bool is_uninitialized() const noexcept { return state_ == details::ObjectStorageState::UNINITIALIZED; }
  constexpr bool is_constructed() const noexcept { return state_ == details::ObjectStorageState::CONSTRUCTED; }
  constexpr bool is_destructed() const noexcept { return state_ == details::ObjectStorageState::DESTRUCTED; }

  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T{std::forward<ArgsT>(args)...};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return *value;
  }

  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T;
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return *value;
  }

  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // For trivially destructible types explicitly calling destruct is not necessary.
      state_ = details::ObjectStorageState::DESTRUCTED;
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
  details::ObjectStorageState state_ : 2;
};

/// @brief Specialization for non-trivial, default constructible types exists to make the storage constexpr
/// in C++17, because placement new is not constexpr in C++17.
template <typename T>
class AlignedObjectStorage<T, false>
{
  static_assert(std::is_default_constructible_v<T>,
                "AlignedObjectStorage requires non-trivial types to be default constructible.");

public:
  using is_trivial = std::false_type;
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  constexpr AlignedObjectStorage() noexcept : state_{details::ObjectStorageState::UNINITIALIZED} {}

  constexpr bool is_uninitialized() const noexcept { return state_ == details::ObjectStorageState::UNINITIALIZED; }
  constexpr bool is_constructed() const noexcept { return state_ == details::ObjectStorageState::CONSTRUCTED; }
  constexpr bool is_destructed() const noexcept { return state_ == details::ObjectStorageState::DESTRUCTED; }

  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    data_ = T{std::forward<ArgsT>(args)...};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return data_;
  }

  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    data_ = T{};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return data_;
  }

  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // No need to explicitly call the destructor, base it will be called either during new value assignment or when
      // during destruction of the storage.
      state_ = details::ObjectStorageState::DESTRUCTED;
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
  details::ObjectStorageState state_ : 2;
};

template <typename T, typename DerivedT>
class GenericStaticContiguousStorage
{
  template <typename ValueRefT, typename ContainerT>
  friend class StaticContiguousStorageIterator;

  friend DerivedT;

public:
  using storage_type = AlignedObjectStorage<T, details::IS_TRIVIAL_V<T>>;
  using value_type = typename storage_type::value_type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = typename storage_type::reference;
  using const_reference = typename storage_type::const_reference;
  using pointer = typename storage_type::pointer;
  using const_pointer = typename storage_type::const_pointer;
  using iterator = StaticContiguousStorageIterator<reference, GenericStaticContiguousStorage>;
  using const_iterator = StaticContiguousStorageIterator<const_reference, const GenericStaticContiguousStorage>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr size_type used_slots() const noexcept { return used_slots_; }
  constexpr size_type size() const noexcept { return used_slots_; }
  constexpr bool empty() const noexcept { return used_slots_ == 0U; }
  constexpr size_type capacity() const noexcept { return capacity_; }

  template <typename... ArgsT>
  constexpr reference construct_at(const size_type index, ArgsT&&... args) noexcept
  {
    assert(!is_constructed(index));
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct(std::forward<ArgsT>(args)...);
    ++used_slots_;
    return value;
  }

  constexpr reference construct_for_overwrite_at(const size_type index) noexcept
  {
    assert(!is_constructed(index));
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct_for_overwrite_at();
    ++used_slots_;
    return value;
  }

  constexpr void destruct_at(const size_type index) noexcept
  {
    assert(is_constructed(index));
    auto& storage = get_derived().get_storage(index);
    storage.destruct();
    --used_slots_;
  }

  constexpr bool is_constructed(const size_type index) const noexcept
  {
    assert(index < capacity());
    const auto& storage = get_derived().get_storage(index);
    return storage.is_constructed();
  }

  constexpr reference operator[](const size_type index) noexcept
  {
    assert(is_constructed(index));
    auto& storage = get_derived().get_storage(index);
    return storage.get_reference();
  }

  constexpr const_reference operator[](const size_type index) const noexcept
  {
    assert(is_constructed(index));
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

  constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
  constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

  constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

private:
  constexpr explicit GenericStaticContiguousStorage(const size_t capacity) noexcept : capacity_{capacity}
  {
    assert(capacity > 0U);
  }

  constexpr DerivedT& get_derived() noexcept { return static_cast<DerivedT&>(*this); }
  constexpr const DerivedT& get_derived() const noexcept { return static_cast<const DerivedT&>(*this); }

  size_type used_slots_{0U};
  size_type capacity_{0U};
};

template <typename T>
class StaticContiguousStorage : public GenericStaticContiguousStorage<T, StaticContiguousStorage<T>>
{
  template <typename ValueRefT, typename ContainerT>
  friend class StaticContiguousStorageIterator;

  using Base = GenericStaticContiguousStorage<T, StaticContiguousStorage<T>>;

  friend Base;

public:
  using storage_type = typename Base::storage_type;
  using size_type = typename Base::size_type;

  explicit StaticContiguousStorage(const size_type capacity) noexcept
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

  std::unique_ptr<storage_type[]> data_;
};

template <typename T, std::size_t CAPACITY>
class InplaceStaticContiguousStorage
    : public GenericStaticContiguousStorage<T, InplaceStaticContiguousStorage<T, CAPACITY>>
{
  template <typename ValueRefT, typename ContainerT>
  friend class StaticContiguousStorageIterator;

  using Base = GenericStaticContiguousStorage<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

  friend Base;

public:
  static_assert(CAPACITY > 0U, "CAPACITY must be greater than 0.");

  using storage_type = typename Base::storage_type;
  using size_type = typename Base::size_type;

  constexpr InplaceStaticContiguousStorage() noexcept : Base{CAPACITY} {}

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

  alignas(storage_type) std::array<storage_type, CAPACITY> data_{};
};

template <typename IteratorT>
constexpr bool is_memory_contiguous(IteratorT begin, IteratorT end) noexcept
{
  bool contiguous = true;

  const auto size = std::distance(begin, end);
  for (std::ptrdiff_t i = 0; i < size; ++i)
  {
    if constexpr (std::is_same_v<typename IteratorT::iterator_category, details::StaticContiguousStorageIteratorTag>)
    {
      // The special treatment is required, because of the AlignedObjectStorage,
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
