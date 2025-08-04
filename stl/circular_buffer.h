#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = ContiguousStorage<T>>
class GenericCircularBuffer
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

  constexpr GenericCircularBuffer() noexcept = default;

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace_back(ArgsT&&... args) noexcept
  {
    const auto index = (head_++) % capacity();
    if (storage_.is_constructed(index))
    {
      storage_.destruct_at(index);
    }
    return storage_.construct_at(index, std::forward<ArgsT>(args)...);
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

  constexpr void clear() noexcept
  {
    head_ = 0U;
    storage_.clear();
  }

  constexpr reference operator[](const size_type index) noexcept { return storage_[index]; }
  constexpr const_reference operator[](const size_type index) const noexcept { return storage_[index]; }

  constexpr iterator begin() noexcept { return storage_.begin(); }
  constexpr const_iterator begin() const noexcept { return storage_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return storage_.cbegin(); }

  constexpr iterator end() noexcept { return storage_.end(); }
  constexpr const_iterator end() const noexcept { return storage_.end(); }
  constexpr const_iterator cend() const noexcept { return storage_.cend(); }

protected:
  constexpr explicit GenericCircularBuffer(const size_type capacity) noexcept : storage_{capacity} {}

private:
  StorageType storage_;
  size_type head_{0U};
};

template <typename T, typename BaseT = GenericCircularBuffer<T, ContiguousStorage<T>>>
class CircularBuffer : public BaseT
{
public:
  explicit CircularBuffer(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

template <typename T, std::size_t CAPACITY>
using InplaceCircularBuffer = GenericCircularBuffer<T, InplaceContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
