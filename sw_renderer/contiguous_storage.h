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

  constexpr AlignedObjectStorage() : constructed_{false} {}
  constexpr AlignedObjectStorage(const AlignedObjectStorage&) = default;
  constexpr AlignedObjectStorage(AlignedObjectStorage&&) = default;
  constexpr AlignedObjectStorage& operator=(const AlignedObjectStorage&) = default;
  constexpr AlignedObjectStorage& operator=(AlignedObjectStorage&&) = default;
  ~AlignedObjectStorage() { destruct(); }

  constexpr bool is_constructed() const { return constructed_; }

  template <typename... ArgsT>
  constexpr void construct(ArgsT&&... args)
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ::new (reinterpret_cast<void*>(data_.data())) T{std::forward<ArgsT>(args)...};
    constructed_ = true;
  }

  constexpr T& construct_for_overwrite_at()
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(data_.data())) T;
    constructed_ = true;
    return *value;
  }

  constexpr void destruct()
  {
    if (is_constructed())
    {
      get_pointer()->~T();
      constructed_ = false;
    }
  }

  constexpr pointer get_pointer()
  {
    assert(is_constructed());
    return std::launder(reinterpret_cast<T*>(data_.data())); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
  }
  constexpr const_pointer get_pointer() const
  {
    assert(is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::launder(reinterpret_cast<const T*>(data_.data()));
  }
  constexpr pointer operator->() { return get_pointer(); }
  constexpr const_pointer operator->() const { return get_pointer(); }

  constexpr reference get_reference() { return *get_pointer(); }
  constexpr const_reference get_reference() const { return *get_pointer(); }
  constexpr reference operator*() { return get_reference(); }
  constexpr const_reference operator*() const { return get_reference(); }

  constexpr const std::byte* get_raw_pointer() const { return data_.data(); }
  constexpr std::size_t get_raw_size() const { return data_.size(); }

private:
  alignas(alignof(T)) std::array<std::byte, sizeof(T)> data_{};
  bool constructed_ : 1;
};

template <typename T>
class ContiguousStorage
{
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

  explicit ContiguousStorage(const size_type capacity)
      : data_{std::make_unique<AlignedStorage[]>(capacity)}, capacity_{capacity}
  {
    assert(capacity > 0U);
  }

  ContiguousStorage(const ContiguousStorage&) = delete;
  ContiguousStorage(ContiguousStorage&&) = default;
  ContiguousStorage& operator=(const ContiguousStorage&) = delete;
  ContiguousStorage& operator=(ContiguousStorage&&) = default;
  ~ContiguousStorage() { clear(); }

  size_type used_slots() const { return used_slots_; }
  bool empty() const { return used_slots_ == 0U; }
  size_type capacity() const { return capacity_; }

  template <typename... ArgsT>
  void construct_at(const size_type index, ArgsT&&... args)
  {
    assert(index < capacity_);
    data_.get()[index].construct(std::forward<ArgsT>(args)...);
    ++used_slots_;
  }

  reference construct_for_overwrite_at(const size_type index)
  {
    assert(index < capacity_);
    auto& value = data_.get()[index].construct_for_overwrite_at();
    ++used_slots_;
    return value;
  }

  void destruct_at(const size_type index)
  {
    assert(index < capacity_);
    assert(!empty());
    data_.get()[index].destruct();
    --used_slots_;
  }

  bool is_constructed(const size_type index) const { return data_.get()[index].is_constructed(); }

  reference operator[](const size_type index)
  {
    assert(index < used_slots_);
    return data_.get()[index].get_reference();
  }
  const_reference operator[](const size_type index) const { return operator[](index); }

  void clear()
  {
    for (size_type index = 0U; index < capacity_; ++index)
    {
      data_.get()[index].destruct();
    }
    used_slots_ = 0U;
  }

  iterator begin() { return data_.get(); }
  const_iterator begin() const { return begin(); }
  const_iterator cbegin() const { return begin(); }

  iterator end() { return begin() + used_slots_; }
  const_iterator end() const { return end(); }
  const_iterator cend() const { return end(); }

private:
  std::unique_ptr<AlignedStorage[]> data_;
  size_type used_slots_{0U};
  size_type capacity_{0U};
};

template <typename IteratorT>
constexpr bool is_memory_contiguous(IteratorT begin, IteratorT end)
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
