#pragma once

#include "math/point.h"

namespace rtw::math {

/// A convex polygon in 2D space.
/// The polygon is represented by a sequence of vertices in counter-clockwise.
/// The last vertex is implicitly connected to the first vertex.
/// TODO: to check if the polygon is convex (no holes, no self-intersection).
/// TODO: to check if vertices are in counter-clockwise order.
/// TODO: to check if the polygon is simple (no self-intersection).
/// TODO: to check if there are no duplicate vertices.
/// @tparam T The type of the polygon's vertices.
/// @tparam Capacity The maximum number of vertices the polygon can hold.
template <typename T, template <typename> typename PointT, std::size_t Capacity>
class ConvexPolygon
{
public:
  static_assert(Capacity >= 3U, "A polygon must have at least capacity of 3.");

  using value_type = PointT<T>;
  using size_type = typename std::array<value_type, Capacity>::size_type;
  using iterator = typename std::array<value_type, Capacity>::iterator;
  using const_iterator = typename std::array<value_type, Capacity>::const_iterator;
  using pointer = typename std::array<value_type, Capacity>::pointer;
  using const_pointer = typename std::array<value_type, Capacity>::const_pointer;
  using reference = typename std::array<value_type, Capacity>::reference;
  using const_reference = typename std::array<value_type, Capacity>::const_reference;

  constexpr ConvexPolygon() {}
  constexpr ConvexPolygon(std::initializer_list<value_type> vertices)
  {
    for (const auto& vertex : vertices)
    {
      push_back(vertex);
    }
  }

  constexpr size_type size() const { return size_; }
  constexpr size_type capacity() const { return Capacity; }

  constexpr const_reference operator[](const std::size_t index) const
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr reference operator[](const std::size_t index)
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr const_reference front() const
  {
    assert(size_ > 0U);
    return vertices_[0U];
  }

  constexpr reference front()
  {
    assert(size_ > 0U);
    return vertices_[0U];
  }

  constexpr const_reference back() const
  {
    assert(size_ > 0U);
    return vertices_[size_ - 1U];
  }

  constexpr reference back()
  {
    assert(size_ > 0U);
    return vertices_[size_ - 1U];
  }

  constexpr const_reference previous(const std::size_t index) const
  {
    assert(index < size_);
    return vertices_[(index + size_ - 1U) % size_];
  }

  constexpr reference previous(const std::size_t index)
  {
    assert(index < size_);
    return vertices_[(index + size_ - 1U) % size_];
  }

  constexpr const_reference at(const std::size_t index) const
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr reference at(const std::size_t index)
  {
    assert(index < size_);
    return vertices_[index];
  }

  constexpr const_reference next(const std::size_t index) const
  {
    assert(index < size_);
    return vertices_[(index + 1U) % size_];
  }

  constexpr reference next(const std::size_t index)
  {
    assert(index < size_);
    return vertices_[(index + 1U) % size_];
  }

  constexpr void push_back(const value_type& vertex)
  {
    assert(size_ + 1U <= Capacity);
    vertices_[size_++] = vertex;
  }

  constexpr void clear() { size_ = 0U; }

  constexpr iterator begin() { return vertices_.begin(); }
  constexpr const_iterator begin() const { return vertices_.begin(); }
  constexpr const_iterator cbegin() const { return vertices_.cbegin(); }
  constexpr iterator end() { return vertices_.begin() + size_; }
  constexpr const_iterator end() const { return vertices_.begin() + size_; }
  constexpr const_iterator cend() const { return vertices_.cbegin() + size_; }

  constexpr bool valid() const { return size_ >= 3U; }

private:
  /// The vertices of the polygon.
  std::array<value_type, Capacity> vertices_;

  /// The number of vertices in the polygon.
  size_type size_{};
};

/// 2D space aliases.
template <typename T, std::size_t Capacity>
using ConvexPolygon2 = ConvexPolygon<T, Point2, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon2f = ConvexPolygon2<float, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon2d = ConvexPolygon2<double, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon2i = ConvexPolygon2<int, Capacity>;

template <typename T>
using Triangle2 = ConvexPolygon2<T, 3>;
using Triangle2f = Triangle2<float>;
using Triangle2d = Triangle2<double>;
using Triangle2i = Triangle2<int>;

template <typename T>
using Quadrilateral2 = ConvexPolygon2<T, 4>;
using Quadrilateral2f = Quadrilateral2<float>;
using Quadrilateral2d = Quadrilateral2<double>;
using Quadrilateral2i = Quadrilateral2<int>;

/// 3D space aliases.
template <typename T, std::size_t Capacity>
using ConvexPolygon3 = ConvexPolygon<T, Point3, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon3f = ConvexPolygon3<float, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon3d = ConvexPolygon3<double, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon3i = ConvexPolygon3<int, Capacity>;

template <typename T>
using Triangle3 = ConvexPolygon3<T, 3>;
using Triangle3f = Triangle3<float>;
using Triangle3d = Triangle3<double>;
using Triangle3i = Triangle3<int>;

template <typename T>
using Quadrilateral3 = ConvexPolygon3<T, 4>;
using Quadrilateral3f = Quadrilateral3<float>;
using Quadrilateral3d = Quadrilateral3<double>;
using Quadrilateral3i = Quadrilateral3<int>;

/// Homogeneous 3D space aliases.
template <typename T, std::size_t Capacity>
using ConvexPolygon4 = ConvexPolygon<T, Point4, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon4f = ConvexPolygon4<float, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon4d = ConvexPolygon4<double, Capacity>;
template <std::size_t Capacity>
using ConvexPolygon4i = ConvexPolygon4<int, Capacity>;

template <typename T>
using Triangle4 = ConvexPolygon4<T, 3>;
using Triangle4f = Triangle4<float>;
using Triangle4d = Triangle4<double>;
using Triangle4i = Triangle4<int>;

template <typename T>
using Quadrilateral4 = ConvexPolygon<T, Point4, 4>;
using Quadrilateral4f = Quadrilateral4<float>;
using Quadrilateral4d = Quadrilateral4<double>;
using Quadrilateral4i = Quadrilateral4<int>;

} // namespace rtw::math
