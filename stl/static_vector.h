#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = ContiguousStorage<T>>
class GenericStaticVector
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;
  using pointer = typename StorageType::pointer;
  using const_pointer = typename StorageType::const_pointer;
  using iterator = typename StorageType::iterator;
  using const_iterator = typename StorageType::const_iterator;

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    return storage_.construct_at(size(), std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push_back(U&& value) noexcept
  {
    emplace_back(std::forward<U>(value));
  }

  constexpr void pop_back() noexcept
  {
    assert(!empty());
    storage_.destruct_at(size() - 1U);
  }

  constexpr void clear() noexcept { storage_.clear(); }

  constexpr reference operator[](const size_type index) noexcept { return storage_[index]; }
  constexpr const_reference operator[](const size_type index) const noexcept { return storage_[index]; }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

protected:
  constexpr explicit GenericStaticVector(const size_type capacity) noexcept : storage_{capacity} {}

private:
  StorageType storage_;
};

template <typename T>
class StaticVector : public GenericStaticVector<T, ContiguousStorage<T>>
{
  using Base = GenericStaticVector<T, ContiguousStorage<T>>;

public:
  using size_type = typename Base::size_type;

  explicit StaticVector(const size_type capacity) noexcept : Base{capacity} {}
};

template <typename T, std::size_t CAPACITY>
class InplaceStaticVector : public GenericStaticVector<T, InplaceContiguousStorage<T, CAPACITY>>
{
  using Base = GenericStaticVector<T, InplaceContiguousStorage<T, CAPACITY>>;

public:
  constexpr InplaceStaticVector() noexcept : Base{CAPACITY} {}
};

} // namespace rtw::stl
