#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = ContiguousStorage<T>>
class GenericQueue
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericQueue() noexcept = default;

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    assert(tail_ + 1U <= capacity());
    tail_ = empty() ? head_ : (tail_ + 1U) % capacity();
    return storage_.construct_at(tail_, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  constexpr void pop(T& value) noexcept
  {
    assert(!empty());
    value = storage_[head_];
    pop_front();
  }

  constexpr void pop() noexcept
  {
    assert(!empty());
    pop_front();
  }

  constexpr reference front() noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  constexpr const_reference front() const noexcept { return front(); }

  constexpr reference back() noexcept
  {
    assert(!empty());
    return storage_[tail_];
  }

  constexpr const_reference back() const noexcept { return back(); }

  constexpr void clear()
  {
    head_ = 0U;
    tail_ = 0U;
    storage_.clear();
  }

protected:
  constexpr explicit GenericQueue(const size_type capacity) noexcept : storage_{capacity} {}

private:
  constexpr void pop_front()
  {
    storage_.destruct_at(head_);
    head_ = (head_ + 1U) % capacity();
  }

  StorageType storage_;
  size_type head_{0U};
  size_type tail_{0U};
};

template <typename T, typename BaseT = GenericQueue<T, ContiguousStorage<T>>>
class Queue : public BaseT
{
public:
  explicit Queue(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

template <typename T, std::size_t CAPACITY>
using InplaceQueue = GenericQueue<T, InplaceContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
