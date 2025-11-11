#pragma once

#include "stl/contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = ContiguousStorage<T>>
class GenericStack
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStack() noexcept = default;

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
    assert(top_ < capacity());
    const auto index = top_;
    ++top_;
    return storage_.construct_at(index, std::forward<ArgsT>(args)...);
  }

  template <typename U = T>
  constexpr void push(U&& value) noexcept
  {
    emplace(std::forward<U>(value));
  }

  constexpr void pop(T& value) noexcept
  {
    value = top();
    pop();
  }

  constexpr void pop() noexcept
  {
    assert(!empty());
    --top_;
    storage_.destruct_at(top_);
  }

  constexpr reference top() noexcept
  {
    assert(!empty());
    return storage_[top_ - 1U];
  }

  constexpr const_reference top() const noexcept { return top(); }

  constexpr reference bottom() noexcept
  {
    assert(!empty());
    return storage_[0U];
  }

  constexpr const_reference bottom() const noexcept { return bottom(); }

  constexpr void clear()
  {
    top_ = 0U;
    storage_.clear();
  }

protected:
  constexpr explicit GenericStack(const size_type capacity) noexcept : storage_{capacity} {}

private:
  StorageType storage_;
  size_type top_{0U};
};

template <typename T, typename BaseT = GenericStack<T, ContiguousStorage<T>>>
class Stack : public BaseT
{
public:
  explicit Stack(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

template <typename T, std::size_t CAPACITY>
using InplaceStack = GenericStack<T, InplaceContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
