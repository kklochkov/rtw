#pragma once

#include "sw_renderer/register_file.h"

#include <cstdint>

namespace rtw::sw_renderer
{

/// CRTP helper for type-safe varyings overlays.
///
/// A derived overlay owns a `RegisterFile<T, CAPACITY>` (the public `regs` member) and adds named, typed
/// accessors over its vec4 slots. `VaryingsBase` supplies the concept arithmetic (`operator+` and scalar `operator*`)
/// by forwarding to `regs`, so the overlay needs no operator boilerplate.
///
/// @code
/// struct LitVaryings : VaryingsBase<LitVaryings, single_precision, 3>
/// {
///   Vector3F world_pos() const { return regs[0].xyz(); }
///   void set_world_pos(const Vector3F& p) { regs[0] = Vector4F{p.x(), p.y(), p.z(), single_precision{0}}; }
///   // ...
/// };
/// @endcode
///
/// @tparam DerivedT The concrete overlay type (CRTP).
/// @tparam T The scalar component type of the register file.
/// @tparam  CAPACITY The number of vec4 slots.
template <typename DerivedT, typename T, std::uint16_t CAPACITY>
struct VaryingsBase
{
  RegisterFile<T, CAPACITY> regs{};

  /// Barton-Nackman trick to generate the concept operators.
  /// @{
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
  /// @}
};

} // namespace rtw::sw_renderer
