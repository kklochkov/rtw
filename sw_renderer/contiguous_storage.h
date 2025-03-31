#pragma once

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
    ::new (reinterpret_cast<void*>(&data_[0U])) T{std::forward<ArgsT>(args)...};
    constructed_ = true;
  }

  constexpr T& construct_for_overwrite_at()
  {
    assert(!is_constructed());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-owning-memory)
    auto* value = ::new (reinterpret_cast<void*>(&data_[0U])) T;
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
    return std::launder(reinterpret_cast<T*>(&data_[0U])); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
  }
  constexpr const_pointer get_pointer() const
  {
    assert(is_constructed());
    return std::launder(reinterpret_cast<const T*>(&data_[0U])); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
  }

  constexpr reference get_reference() { return *get_pointer(); }
  constexpr const_reference get_reference() const { return get_reference(); }

  constexpr std::byte* get_raw_pointer() { return &data_[0U]; }

  constexpr std::size_t get_raw_size() const { return sizeof(T); }

private:
  alignas(alignof(T)) std::byte data_[sizeof(T)]{};
  bool constructed_ : 1;
};

template <typename T>
class AlignedObjectStorageIterator
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;

  constexpr explicit AlignedObjectStorageIterator(AlignedObjectStorage<T>* ptr) : ptr_{ptr} {}

  constexpr reference operator*() { return ptr_->get_reference(); }
  constexpr pointer operator->() { return ptr_->get_pointer(); }

  constexpr AlignedObjectStorageIterator& operator++()
  {
    ++ptr_;
    return *this;
  }

  constexpr AlignedObjectStorageIterator operator++(int)
  {
    auto temp = *this;
    ++(*this);
    return temp;
  }

  constexpr bool operator==(const AlignedObjectStorageIterator& other) const { return ptr_ == other.ptr_; }
  constexpr bool operator!=(const AlignedObjectStorageIterator& other) const { return !(*this == other); }

private:
  AlignedObjectStorage<T>* ptr_{nullptr};
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
  using iterator = AlignedObjectStorageIterator<T>;
  using const_iterator = AlignedObjectStorageIterator<const T>;

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
    data_[index].construct(std::forward<ArgsT>(args)...);
    ++used_slots_;
  }

  T& construct_for_overwrite_at(const size_type index)
  {
    assert(index < capacity_);
    auto& value = data_[index].construct_for_overwrite_at();
    ++used_slots_;
    return value;
  }

  void destruct_at(const size_type index)
  {
    assert(index < capacity_);
    assert(!empty());
    data_[index].destruct();
    --used_slots_;
  }

  bool is_constructed(const size_type index) const { return data_[index].is_constructed(); }

  pointer get_pointer(const size_type index)
  {
    assert(index < used_slots_);
    return data_[index].get_pointer();
  }

  reference get_reference(const size_type index) { return *get_pointer(index); }

  reference operator[](const size_type index) { return get_reference(index); }
  const_reference operator[](const size_type index) const { return get_reference(index); }

  void clear()
  {
    for (size_type index = 0U; index < capacity_; ++index)
    {
      data_[index].destruct();
    }
    used_slots_ = 0U;
  }

  iterator begin() { return iterator{&data_[0U]}; }
  const_iterator begin() const { return const_iterator{&data_[0U]}; }
  const_iterator cbegin() const { return begin(); }

  iterator end() { return iterator{&data_[0U] + used_slots_}; }
  const_iterator end() const { return const_iterator{&data_[0U] + used_slots_}; }
  const_iterator cend() const { return end(); }

private:
  std::unique_ptr<AlignedStorage[]> data_;
  size_type used_slots_{0U};
  size_type capacity_{0U};
};

} // namespace rtw::stl
