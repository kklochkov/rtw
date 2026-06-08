#pragma once

#include "stl/contiguous_storage_iterator.h"

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

/// @brief Tri-state lifecycle tracking for storage slots.
/// UNINITIALIZED: slot has never held a value (or was reset via clear()).
/// CONSTRUCTED: slot currently holds a live value.
/// DESTRUCTED: slot previously held a value that was erased (tombstone for hash probing).
enum class ObjectStorageState : std::uint8_t
{
  UNINITIALIZED = 0U,
  CONSTRUCTED,
  DESTRUCTED
};

} // namespace details

/// @brief Type-erased aligned storage for a single object with tri-state lifecycle tracking.
///
/// AlignedObjectStorage<T, true> (trivial types): uses placement-new into a byte array.
/// AlignedObjectStorage<T, false> (non-trivial, default-constructible types): uses assignment
/// for C++17 constexpr compatibility (placement-new is not constexpr until C++26).
///
/// Tracks object state as UNINITIALIZED / CONSTRUCTED / DESTRUCTED to support tombstone-aware
/// probing in hash containers.
template <typename T, bool IS_TRIVIAL>
class AlignedObjectStorage;

/// @brief Specialization for trivial types: uses placement-new into an aligned byte array.
/// Supports non-constexpr construction but benefits from trivial copy/destruction semantics.
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

  /// @brief Constructs the stored object in-place via placement-new.
  /// @param[in] args Arguments forwarded to T's constructor.
  /// @return Reference to the constructed object.
  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T{std::forward<ArgsT>(args)...};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return *value;
  }

  /// @brief Constructs the stored object via default-initialization (no arguments).
  /// @return Reference to the constructed object.
  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T;
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return *value;
  }

  /// @brief Marks the slot as destructed (tombstone). For trivial types, no destructor is called.
  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // For trivially destructible types explicitly calling destruct is not necessary.
      state_ = details::ObjectStorageState::DESTRUCTED;
    }
  }

  /// @brief Resets the slot to UNINITIALIZED state (used by clear()).
  constexpr void reset() noexcept
  {
    destruct();
    state_ = details::ObjectStorageState::UNINITIALIZED;
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

  /// @brief Constructs the stored object via assignment (constexpr-compatible in C++17).
  /// @param[in] args Arguments forwarded to T's brace-init constructor.
  /// @return Reference to the constructed object.
  template <typename... ArgsT>
  constexpr reference construct(ArgsT&&... args) noexcept
  {
    assert(!is_constructed());
    data_ = T{std::forward<ArgsT>(args)...};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return data_;
  }

  /// @brief Constructs the stored object via default-initialization.
  /// @return Reference to the constructed object.
  constexpr reference construct_for_overwrite_at() noexcept
  {
    assert(!is_constructed());
    data_ = T{};
    state_ = details::ObjectStorageState::CONSTRUCTED;
    return data_;
  }

  /// @brief Marks the slot as destructed (tombstone). Does not call the destructor explicitly.
  constexpr void destruct() noexcept
  {
    if (is_constructed())
    {
      // No need to explicitly call the destructor, base it will be called either during new value assignment or when
      // during destruction of the storage.
      state_ = details::ObjectStorageState::DESTRUCTED;
    }
  }

  /// @brief Resets the slot to UNINITIALIZED state (used by clear()).
  constexpr void reset() noexcept
  {
    destruct();
    state_ = details::ObjectStorageState::UNINITIALIZED;
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

/// @brief CRTP base providing indexed construct/destruct/clear over a contiguous array of AlignedObjectStorage slots.
///
/// Derived classes (StaticContiguousStorage, InplaceStaticContiguousStorage) supply the actual
/// backing array via get_storage(index). This base manages the used_slots count and provides
/// iterators, operator[], and bulk clear().
///
/// @tparam T Element type.
/// @tparam DerivedT CRTP derived class that implements get_storage().
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

  constexpr bool is_uninitialized(const size_type index) const noexcept
  {
    assert(index < capacity());
    const auto& storage = get_derived().get_storage(index);
    return storage.is_uninitialized();
  }

  constexpr bool is_constructed(const size_type index) const noexcept
  {
    assert(index < capacity());
    const auto& storage = get_derived().get_storage(index);
    return storage.is_constructed();
  }

  constexpr bool is_destructed(const size_type index) const noexcept
  {
    assert(index < capacity());
    const auto& storage = get_derived().get_storage(index);
    return storage.is_destructed();
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
      storage.reset();
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

  /// @brief Returns a pointer to the underlying storage of the first slot.
  /// Valid even when the container is empty (returns the address of slot 0).
  constexpr pointer data() noexcept { return get_derived().get_storage(0U).get_pointer(); }

  /// @brief Returns a const pointer to the underlying storage of the first slot.
  /// Valid even when the container is empty (returns the address of slot 0).
  constexpr const_pointer data() const noexcept { return get_derived().get_storage(0U).get_pointer(); }

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
