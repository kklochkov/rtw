#pragma once

#include "math/point.h"
#include "math/vector_operations.h"

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
using ConvexPolygon2F = ConvexPolygon2<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2D = ConvexPolygon2<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2I = ConvexPolygon2<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2Q16 = ConvexPolygon2<multiprecision::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon2Q32 = ConvexPolygon2<multiprecision::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle2 = ConvexPolygon2<T, 3>;
using Triangle2F = Triangle2<float>;
using Triangle2D = Triangle2<double>;
using Triangle2I = Triangle2<int>;
using Triangle2Q16 = Triangle2<multiprecision::FixedPoint16>;
using Triangle2Q32 = Triangle2<multiprecision::FixedPoint32>;

template <typename T>
using Quadrilateral2 = ConvexPolygon2<T, 4>;
using Quadrilateral2F = Quadrilateral2<float>;
using Quadrilateral2D = Quadrilateral2<double>;
using Quadrilateral2I = Quadrilateral2<int>;
using Quadrilateral2Q16 = Quadrilateral2<multiprecision::FixedPoint16>;
using Quadrilateral2Q32 = Quadrilateral2<multiprecision::FixedPoint32>;

/// 3D space aliases.
template <typename T, std::size_t CAPACITY>
using ConvexPolygon3 = ConvexPolygon<T, Point3, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3F = ConvexPolygon3<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3D = ConvexPolygon3<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3I = ConvexPolygon3<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3Q16 = ConvexPolygon3<multiprecision::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon3Q32 = ConvexPolygon3<multiprecision::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle3 = ConvexPolygon3<T, 3>;
using Triangle3F = Triangle3<float>;
using Triangle3D = Triangle3<double>;
using Triangle3I = Triangle3<int>;
using Triangle3Q16 = Triangle3<multiprecision::FixedPoint16>;
using Triangle3Q32 = Triangle3<multiprecision::FixedPoint32>;

template <typename T>
using Quadrilateral3 = ConvexPolygon3<T, 4>;
using Quadrilateral3F = Quadrilateral3<float>;
using Quadrilateral3D = Quadrilateral3<double>;
using Quadrilateral3I = Quadrilateral3<int>;
using Quadrilateral3Q16 = Quadrilateral3<multiprecision::FixedPoint16>;
using Quadrilateral3Q32 = Quadrilateral3<multiprecision::FixedPoint32>;

/// Homogeneous 3D space aliases.
template <typename T, std::size_t CAPACITY>
using ConvexPolygon4 = ConvexPolygon<T, Point4, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4F = ConvexPolygon4<float, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4D = ConvexPolygon4<double, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4I = ConvexPolygon4<int, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4Q16 = ConvexPolygon4<multiprecision::FixedPoint16, CAPACITY>;
template <std::size_t CAPACITY>
using ConvexPolygon4Q32 = ConvexPolygon4<multiprecision::FixedPoint32, CAPACITY>;

template <typename T>
using Triangle4 = ConvexPolygon4<T, 3>;
using Triangle4F = Triangle4<float>;
using Triangle4D = Triangle4<double>;
using Triangle4I = Triangle4<int>;
using Triangle4Q16 = Triangle4<multiprecision::FixedPoint16>;
using Triangle4Q32 = Triangle4<multiprecision::FixedPoint32>;

template <typename T>
using Quadrilateral4 = ConvexPolygon<T, Point4, 4>;
using Quadrilateral4F = Quadrilateral4<float>;
using Quadrilateral4D = Quadrilateral4<double>;
using Quadrilateral4I = Quadrilateral4<int>;
using Quadrilateral4Q16 = Quadrilateral4<multiprecision::FixedPoint16>;
using Quadrilateral4Q32 = Quadrilateral4<multiprecision::FixedPoint32>;

enum class WindingOrder : std::uint8_t
{
  COUNTER_CLOCKWISE,
  CLOCKWISE
};

/// Check the winding order of a triangle.
/// @tparam T The type of the elements.
/// @param[in] v0 The first vertex of the triangle.
/// @param[in] v1 The second vertex of the triangle.
/// @param[in] v2 The third vertex of the triangle.
/// @return The winding order of the triangle.
template <typename T>
constexpr WindingOrder winding_order(const Point2<T>& v0, const Point2<T>& v1, const Point2<T>& v2) noexcept
{
  return cross(v1 - v0, v2 - v1) > T{0} ? WindingOrder::COUNTER_CLOCKWISE : WindingOrder::CLOCKWISE;
}

template <typename T>
constexpr WindingOrder winding_order(const Triangle2<T>& triangle) noexcept
{
  return winding_order(triangle[0U], triangle[1U], triangle[2U]);
}

} // namespace rtw::math
