#pragma once

#include "stl/static_contiguous_storage.h"

namespace rtw::stl
{

template <typename T, typename StorageT = StaticContiguousStorage<T>>
class GenericStaticStack
{
  using StorageType = StorageT;

public:
  using value_type = typename StorageType::value_type;
  using size_type = typename StorageType::size_type;
  using reference = typename StorageType::reference;
  using const_reference = typename StorageType::const_reference;

  constexpr GenericStaticStack() noexcept = default;
  constexpr explicit GenericStaticStack(const size_type capacity) noexcept : storage_{capacity} {}

  constexpr size_type size() const noexcept { return storage_.used_slots(); }
  constexpr bool empty() const noexcept { return storage_.empty(); }
  constexpr size_type capacity() const noexcept { return storage_.capacity(); }

  template <typename... ArgsT>
  constexpr reference emplace(ArgsT&&... args) noexcept
  {
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
    --top_;
    storage_.destruct_at(top_);
  }

  constexpr reference top() noexcept { return storage_[top_ - 1U]; }
  constexpr const_reference top() const noexcept { return top(); }

  constexpr reference bottom() noexcept { return storage_[0U]; }
  constexpr const_reference bottom() const noexcept { return bottom(); }

  constexpr void clear()
  {
    top_ = 0U;
    storage_.clear();
  }

private:
  StorageType storage_;
  size_type top_{0U};
};

template <typename T>
using StaticStack = GenericStaticStack<T, StaticContiguousStorage<T>>;

template <typename T, std::size_t CAPACITY>
using InplaceStaticStack = GenericStaticStack<T, InplaceStaticContiguousStorage<T, CAPACITY>>;

} // namespace rtw::stl
