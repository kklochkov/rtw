#pragma once

#include <array>
#include <cassert>
#include <memory>

namespace rtw::stl
{

template <typename T>
class AlignedObjectStorage
{
public:
  static_assert(std::is_standard_layout_v<T>, "AlignedObjectStorage requires T to be standard layout.");
  static_assert(std::is_trivially_copyable_v<T>, "AlignedObjectStorage requires T to be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<T>, "AlignedObjectStorage requires T to be trivially destructible.");

  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  constexpr AlignedObjectStorage() noexcept : constructed_{false} {}
  constexpr AlignedObjectStorage(const AlignedObjectStorage&) noexcept = default;
  constexpr AlignedObjectStorage(AlignedObjectStorage&&) noexcept = default;
  constexpr AlignedObjectStorage& operator=(const AlignedObjectStorage&) noexcept = default;
  constexpr AlignedObjectStorage& operator=(AlignedObjectStorage&&) noexcept = default;
  ~AlignedObjectStorage() noexcept { destruct(); }

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
      get_pointer()->~T();
      constructed_ = false;
    }
  }

  constexpr pointer get_pointer() noexcept
  {
    assert(is_constructed());
    return std::launder(reinterpret_cast<T*>(data_.data())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
  }
  constexpr const_pointer get_pointer() const noexcept
  {
    assert(is_constructed());
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

template <typename T, typename DerivedT>
class GenericContiguousStorage
{
protected:
  using AlignedStorage = AlignedObjectStorage<T>;

public:
  static_assert(std::is_standard_layout_v<T>, " ContiguousStorage requires T to be standard layout.");
  static_assert(std::is_trivially_copyable_v<T>, " ContiguousStorage requires T to be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<T>, " ContiguousStorage requires T to be trivially destructible.");

  using value_type = typename AlignedStorage::value_type;
  using size_type = std::size_t;
  using reference = typename AlignedStorage::reference;
  using const_reference = typename AlignedStorage::const_reference;
  using pointer = typename AlignedStorage::pointer;
  using const_pointer = typename AlignedStorage::const_pointer;
  using iterator = AlignedStorage*;
  using const_iterator = const AlignedStorage*;

constexpr  size_type used_slots() const noexcept { return used_slots_; }
constexpr  bool empty() const noexcept { return used_slots_ == 0U; }
constexpr  size_type capacity() const noexcept { return capacity_; }

  template <typename... ArgsT>
constexpr   reference construct_at(const size_type index, ArgsT&&... args) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct(std::forward<ArgsT>(args)...);
    ++used_slots_;
    return value;
  }

constexpr   reference construct_for_overwrite_at(const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    auto& value = storage.construct_for_overwrite_at();
    ++used_slots_;
    return value;
  }

constexpr   void destruct_at(const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    storage.destruct();
    --used_slots_;
  }

constexpr   bool is_constructed(const size_type index) const noexcept
  {
    const auto& storage = get_derived().get_storage(index);
    return storage.is_constructed();
  }

constexpr   reference operator[](const size_type index) noexcept
  {
    auto& storage = get_derived().get_storage(index);
    return storage.get_reference();
  }

constexpr   const_reference operator[](const size_type index) const noexcept
  {
    const auto& storage = get_derived().get_storage(index);
    return storage.get_reference();
  }

constexpr   void clear() noexcept
  {
    for (size_type index = 0U; index < capacity_; ++index)
    {
      auto& storage = get_derived().get_storage(index);
      storage.destruct();
    }
    used_slots_ = 0U;
  }

  constexpr iterator begin() noexcept { return get_derived().get_storage(); }
  constexpr const_iterator begin() const noexcept { return get_derived().get_storage(); }
  constexpr const_iterator cbegin() const noexcept { return begin(); }

  constexpr iterator end() noexcept { return begin() + capacity_; }
  constexpr const_iterator end() const noexcept { return begin() + capacity_; }
  constexpr const_iterator cend() const noexcept { return end(); }

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

template <typename T>
class ContiguousStorage : public GenericContiguousStorage<T, ContiguousStorage<T>>
{
  using Base = GenericContiguousStorage<T, ContiguousStorage<T>>;
  using AlignedStorage = typename Base::AlignedStorage;

  friend Base;

public:
  using size_type = typename Base::size_type;

  explicit ContiguousStorage(const size_type capacity) noexcept
      : Base{capacity}, data_{std::make_unique<AlignedStorage[]>(capacity)}
  {
  }

  ContiguousStorage(const ContiguousStorage&) noexcept = delete;
  ContiguousStorage(ContiguousStorage&&) noexcept = default;
  ContiguousStorage& operator=(const ContiguousStorage&) noexcept = delete;
  ContiguousStorage& operator=(ContiguousStorage&&) noexcept = default;
  ~ContiguousStorage() = default;

private:
  AlignedStorage& get_storage(const size_type index) noexcept
  {
    assert(index < Base::capacity());
    return data_.get()[index];
  }

  const AlignedStorage& get_storage(const size_type index) const noexcept
  {
    assert(index < Base::capacity());
    return data_.get()[index];
  }

  AlignedStorage* get_storage() noexcept { return data_.get(); }
  const AlignedStorage* get_storage() const noexcept { return data_.get(); }

  std::unique_ptr<AlignedStorage[]> data_;
};

template <typename T, std::size_t CAPACITY>
class InplaceContiguousStorage : public GenericContiguousStorage<T, ContiguousStorage<T>>
{
  using Base = GenericContiguousStorage<T, ContiguousStorage<T>>;
  using AlignedStorage = typename Base::AlignedStorage;

  friend Base;

public:
  using size_type = typename Base::size_type;

  constexpr InplaceContiguousStorage() noexcept : Base{CAPACITY} {}

private:
  constexpr AlignedStorage& get_storage(const size_type index) noexcept
  {
    assert(index < Base::capacity());
    return data_[index];
  }

  constexpr const AlignedStorage& get_storage(const size_type index) const noexcept
  {
    assert(index < Base::capacity());
    return data_[index];
  }

  constexpr AlignedStorage* get_storage() noexcept { return data_.data(); }
  constexpr const AlignedStorage* get_storage() const noexcept { return data_.data(); }

  alignas(AlignedStorage) std::array<AlignedStorage, CAPACITY> data_{};
};

template <typename IteratorT>
constexpr bool is_memory_contiguous(IteratorT begin, IteratorT end) noexcept
{
  bool contiguous = true;

  const auto size = std::distance(begin, end);
  for (std::ptrdiff_t i = 0U; i < size; ++i)
  {
    const auto* a = &*(std::next(begin, i));
    const auto* b = &*(std::next(&*begin, i));
    contiguous &= a == b;
  }

  return contiguous;
}

} // namespace rtw::stl
