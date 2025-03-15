#pragma once

#include "math/point.h"

namespace rtw::math
{

/// A convex polygon in 2D space.
/// The polygon is represented by a sequence of vertices in counter-clockwise.
/// The last vertex is implicitly connected to the first vertex.
/// TODO: to check if the polygon is convex (no holes, no self-intersection).
/// TODO: to check if vertices are in counter-clockwise order.
/// TODO: to check if the polygon is simple (no self-intersection).
/// TODO: to check if there are no duplicate vertices.
/// @tparam T The type of the polygon's vertices.
/// @tparam cAPACITY The maximum number of vertices the polygon can hold.
template <typename T, template <typename> typename PointT, std::size_t CAPACITY>
class ConvexPolygon
{
public:
  static_assert(CAPACITY >= 3U, "A polygon must have at least capacity of 3.");

  using value_type = PointT<T>;
  using size_type = typename std::array<value_type, CAPACITY>::size_type;
  using iterator = typename std::array<value_type, CAPACITY>::iterator;
  using const_iterator = typename std::array<value_type, CAPACITY>::const_iterator;
  using pointer = typename std::array<value_type, CAPACITY>::pointer;
  using const_pointer = typename std::array<value_type, CAPACITY>::const_pointer;
  using reference = typename std::array<value_type, CAPACITY>::reference;
  using const_reference = typename std::array<value_type, CAPACITY>::const_reference;

  constexpr ConvexPolygon() noexcept = default;
  constexpr ConvexPolygon(std::initializer_list<value_type> vertices) noexcept
  {
    for (const auto& vertex : vertices)
    {
      push_back(vertex);
    }
  }

  constexpr size_type size() const noexcept { return size_; }
  constexpr size_type capacity() const noexcept { return CAPACITY; }

  constexpr const_reference operator[](const std::size_t index) const noexcept
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr reference operator[](const std::size_t index) noexcept
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr const_reference front() const noexcept
  {
    assert(size_ > 0U);
    return vertices_[0U];
  }

  constexpr reference front() noexcept
  {
    assert(size_ > 0U);
    return vertices_[0U];
  }

  constexpr const_reference back() const noexcept
  {
    assert(size_ > 0U);
    return vertices_[size_ - 1U];
  }

  constexpr reference back() noexcept
  {
    assert(size_ > 0U);
    return vertices_[size_ - 1U];
  }

  constexpr const_reference previous(const std::size_t index) const noexcept
  {
    assert(index < size_);
    return vertices_[(index + size_ - 1U) % size_];
  }

  constexpr reference previous(const std::size_t index) noexcept
  {
    assert(index < size_);
    return vertices_[(index + size_ - 1U) % size_];
  }

  constexpr const_reference at(const std::size_t index) const noexcept
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr reference at(const std::size_t index) noexcept
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr const_reference next(const std::size_t index) const noexcept
  {
    assert(index < size_);
    return vertices_[(index + 1U) % size_];
  }

  constexpr reference next(const std::size_t index) noexcept
  {
    assert(index < size_);
    return vertices_[(index + 1U) % size_];
  }

  constexpr void push_back(const value_type& vertex) noexcept
  {
    assert(size_ + 1U <= CAPACITY);
    vertices_[size_++] = vertex;
  }

  constexpr void clear() noexcept { size_ = 0U; }

  constexpr iterator begin() noexcept { return vertices_.begin(); }
  constexpr const_iterator begin() const noexcept { return vertices_.begin(); }
  constexpr const_iterator cbegin() const noexcept { return vertices_.cbegin(); }
  constexpr iterator end() noexcept { return vertices_.begin() + size_; }
  constexpr const_iterator end() const noexcept { return vertices_.begin() + size_; }
  constexpr const_iterator cend() const noexcept { return vertices_.cbegin() + size_; }

  constexpr bool valid() const noexcept { return size_ >= 3U; }

private:
  /// The vertices of the polygon.
  std::array<value_type, CAPACITY> vertices_;

  /// The number of vertices in the polygon.
  size_type size_{};
};

/// 2D space aliases.
template <typename T, std::size_t CAPACITY>
using ConvexPolygon2 = ConvexPolygon<T, Point2, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2f = ConvexPolygon2<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2d = ConvexPolygon2<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2i = ConvexPolygon2<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2q16 = ConvexPolygon2<fixed_point::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2q32 = ConvexPolygon2<fixed_point::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle2 = ConvexPolygon2<T, 3>;
using Triangle2f = Triangle2<float>;
using Triangle2d = Triangle2<double>;
using Triangle2i = Triangle2<int>;
using Triangle2q16 = Triangle2<fixed_point::FixedPoint16>;
using Triangle2q32 = Triangle2<fixed_point::FixedPoint32>;

template <typename T>
using Quadrilateral2 = ConvexPolygon2<T, 4>;
using Quadrilateral2f = Quadrilateral2<float>;
using Quadrilateral2d = Quadrilateral2<double>;
using Quadrilateral2i = Quadrilateral2<int>;
using Quadrilateral2q16 = Quadrilateral2<fixed_point::FixedPoint16>;
using Quadrilateral2q32 = Quadrilateral2<fixed_point::FixedPoint32>;

/// 3D space aliases.
template <typename T, std::size_t CAPACITY>
using ConvexPolygon3 = ConvexPolygon<T, Point3, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3f = ConvexPolygon3<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3d = ConvexPolygon3<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3i = ConvexPolygon3<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3q16 = ConvexPolygon3<fixed_point::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3q32 = ConvexPolygon3<fixed_point::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle3 = ConvexPolygon3<T, 3>;
using Triangle3f = Triangle3<float>;
using Triangle3d = Triangle3<double>;
using Triangle3i = Triangle3<int>;
using Triangle3q16 = Triangle3<fixed_point::FixedPoint16>;
using Triangle3q32 = Triangle3<fixed_point::FixedPoint32>;

template <typename T>
using Quadrilateral3 = ConvexPolygon3<T, 4>;
using Quadrilateral3f = Quadrilateral3<float>;
using Quadrilateral3d = Quadrilateral3<double>;
using Quadrilateral3i = Quadrilateral3<int>;
using Quadrilateral3q16 = Quadrilateral3<fixed_point::FixedPoint16>;
using Quadrilateral3q32 = Quadrilateral3<fixed_point::FixedPoint32>;

/// Homogeneous 3D space aliases.
template <typename T, std::size_t CAPACITY>
using ConvexPolygon4 = ConvexPolygon<T, Point4, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4f = ConvexPolygon4<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4d = ConvexPolygon4<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4i = ConvexPolygon4<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4q16 = ConvexPolygon4<fixed_point::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4q32 = ConvexPolygon4<fixed_point::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle4 = ConvexPolygon4<T, 3>;
using Triangle4f = Triangle4<float>;
using Triangle4d = Triangle4<double>;
using Triangle4i = Triangle4<int>;
using Triangle4q16 = Triangle4<fixed_point::FixedPoint16>;
using Triangle4q32 = Triangle4<fixed_point::FixedPoint32>;

template <typename T>
using Quadrilateral4 = ConvexPolygon<T, Point4, 4>;
using Quadrilateral4f = Quadrilateral4<float>;
using Quadrilateral4d = Quadrilateral4<double>;
using Quadrilateral4i = Quadrilateral4<int>;
using Quadrilateral4q16 = Quadrilateral4<fixed_point::FixedPoint16>;
using Quadrilateral4q32 = Quadrilateral4<fixed_point::FixedPoint32>;

} // namespace rtw::math
