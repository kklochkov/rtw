#pragma once

#include "sw_renderer/programmable_pipeline/register_file.h"

#include <cstdint>

namespace rtw::sw_renderer
{

template <typename DerivedT, typename T, std::uint16_t CAPACITY>
struct VaryingsBase
{
  RegisterFile<T, CAPACITY> regs{};

  friend constexpr DerivedT operator+(DerivedT lhs, const DerivedT& rhs) noexcept
  {
    lhs.regs += rhs.regs;
    return lhs;
  }

  friend constexpr DerivedT operator*(DerivedT lhs, const T rhs) noexcept
  {
    lhs.regs *= rhs;
    return lhs;
  }

  friend constexpr DerivedT operator*(const T lhs, DerivedT rhs) noexcept
  {
    rhs.regs *= lhs;
    return rhs;
  }
};

} // namespace rtw::sw_renderer
