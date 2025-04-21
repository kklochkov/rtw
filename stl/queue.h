#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T>
class Queue
{
  using StorageType = ContiguousStorage<T>;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  explicit Queue(const size_type capacity) noexcept : storage_{capacity} {}

  size_type size() const noexcept { return storage_.used_slots(); }
  bool empty() const noexcept { return storage_.empty(); }
  size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  reference emplace(ArgsT&&... args) noexcept
  {
    assert(tail_ + 1U <= capacity());
    tail_ = empty() ? head_ : (tail_ + 1U) % capacity();
    return storage_.construct_at(tail_, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  void pop(T& value) noexcept
  {
    assert(!empty());
    value = storage_[head_];
    pop_front();
  }

  void pop() noexcept
  {
    assert(!empty());
    pop_front();
  }

  reference front() noexcept
  {
    assert(!empty());
    return storage_[head_];
  }

  const_reference front() const noexcept { return front(); }

  reference back() noexcept
  {
    assert(!empty());
    return storage_[tail_];
  }

  const_reference back() const noexcept { return back(); }

  void clear()
  {
    head_ = 0U;
    tail_ = 0U;
    storage_.clear();
  }

private:
  void pop_front()
  {
    storage_.destruct_at(head_);
    head_ = (head_ + 1U) % capacity();
  }

  StorageType storage_;
  size_type head_{0U};
  size_type tail_{0U};
};

} // namespace rtw::stl
