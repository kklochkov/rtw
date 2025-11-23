#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticQueue
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStaticQueue() noexcept = default;
  constexpr explicit GenericStaticQueue(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    tail_ = empty() ? 0U : (tail_ + 1U) % capacity();
    return storage_.construct_at(tail_, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  constexpr void pop(T& value) noexcept
  {
    value = storage_[head_];
    pop();
  }

  constexpr void pop() noexcept
  {
    storage_.destruct_at(head_);
    head_ = empty() ? 0U : (head_ + 1U) % capacity();
  }

  constexpr reference front() noexcept { return storage_[head_]; }
  constexpr const_reference front() const noexcept { return front(); }

  constexpr reference back() noexcept { return storage_[tail_]; }
  constexpr const_reference back() const noexcept { return back(); }

  constexpr void clear()
  {
    head_ = 0U;
    tail_ = 0U;
    storage_.clear();
  }

private:
  StorageType storage_;
  size_type head_{0U};
  size_type tail_{0U};
};

template <typename T>
using StaticQueue = GenericStaticQueue<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceStaticQueue = GenericStaticQueue<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
