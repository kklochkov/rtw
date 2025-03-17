#pragma once

#include "math/vector.h"

namespace rtw::math
{

/// A plane in Hessian normal form.
/// The normal vector is pointing towards the positive half-space, i.e. the inside of the plane.
/// The distance is the distance from the origin to the plane and is positive.
/// @tparam T The type of the plane elements.
template <typename T>
struct Plane3
{
  Vector3<T> normal{};
  T distance{};
};

using Plane3F = Plane3<float>;
using Plane3D = Plane3<double>;

} // namespace rtw::math
