#pragma once

#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/types.h"

#include "math/interpolation.h"
#include "math/vector.h"
#include "math/vector_operations.h"

#include "multiprecision/fixed_point.h"
#include "multiprecision/fixed_point_math.h"

#include <cstdint>

namespace rtw::sw_renderer
{

constexpr Vector4F texture(const Sampler2D& sampler, const Vector2F& uv) { return sampler.sample(uv); }

template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T mix(const T x, const T y, const T a) noexcept
{
  return math::lerp(x, y, a);
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> mix(const math::Vector<T, N>& x, const math::Vector<T, N>& y, const T a) noexcept
{
  return math::Vector<T, N>{math::lerp(x.as_matrix(), y.as_matrix(), a)};
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> mix(const math::Vector<T, N>& x, const math::Vector<T, N>& y,
                                 const math::Vector<T, N>& a) noexcept
{
  math::Vector<T, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < x.size(); ++i)
  {
    result[i] = mix(x[i], y[i], a[i]);
  }
  return result;
}

template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T saturate(const T value) noexcept
{
  return math::saturate(value);
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> saturate(const math::Vector<T, N>& value) noexcept
{
  return math::saturate(value);
}

template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T step(const T edge, const T x) noexcept
{
  constexpr std::array<T, 2U> VALUES = {T{1}, T{0}};
  return VALUES[static_cast<std::size_t>(x < edge)];
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> step(const math::Vector<T, N>& edge, const math::Vector<T, N>& x) noexcept
{
  math::Vector<T, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < edge.size(); ++i)
  {
    result[i] = step(edge[i], x[i]);
  }
  return result;
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> step(const T& edge, const math::Vector<T, N>& x) noexcept
{
  math::Vector<T, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < x.size(); ++i)
  {
    result[i] = step(edge, x[i]);
  }
  return result;
}

template <typename T, typename = std::enable_if_t<multiprecision::IS_ARITHMETIC_V<T>>>
constexpr T smoothstep(const T edge0, const T edge1, const T x) noexcept
{
  const T t = saturate((x - edge0) / (edge1 - edge0));
  return t * t * (T{3} - T{2} * t);
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> smoothstep(const math::Vector<T, N>& edge0, const math::Vector<T, N>& edge1,
                                        const math::Vector<T, N>& x) noexcept
{
  math::Vector<T, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < edge0.size(); ++i)
  {
    result[i] = smoothstep(edge0[i], edge1[i], x[i]);
  }
  return result;
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> smoothstep(const T edge0, const T edge1, const math::Vector<T, N>& x) noexcept
{
  math::Vector<T, N> result{math::UNINITIALIZED};
  for (std::uint16_t i = 0U; i < x.size(); ++i)
  {
    result[i] = smoothstep(edge0, edge1, x[i]);
  }
  return result;
}

template <typename T>
constexpr T fract(const T value) noexcept
{
  return multiprecision::math::fract(value);
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> fract(const math::Vector<T, N>& value) noexcept
{
  return math::fract(value);
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> reflect(const math::Vector<T, N>& incident, const math::Vector<T, N>& normal) noexcept
{
  return incident - T{2} * math::dot(normal, incident) * normal;
}

template <typename T, std::uint16_t N>
constexpr math::Vector<T, N> refract(const math::Vector<T, N>& incident, const math::Vector<T, N>& normal,
                                     const T eta) noexcept
{
  const T cos_i = math::dot(normal, incident);
  const T k = T{1} - eta * eta * (T{1} - cos_i * cos_i);
  if (k < T{0})
  {
    return math::Vector<T, N>{};
  }
  using multiprecision::math::sqrt;
  using std::sqrt;
  return eta * incident - (eta * cos_i + sqrt(k)) * normal;
}

} // namespace rtw::sw_renderer
