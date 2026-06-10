# rtw/math

A constexpr-first, header-only linear algebra and geometry library for C++17.

Designed for real-time applications (games, renderers, robotics) where compile-time evaluation, fixed-point arithmetic, and zero-overhead abstractions are critical.

## Design Principles

- **Constexpr by default** -- every mathematical function is `constexpr`. Compile-time evaluation is a first-class goal.
- **Type-generic** -- all types are templated and work with `float`, `double`, `int32_t`, `FixedPoint16`, and `FixedPoint32`.
- **Value semantics** -- no raw pointers, no dynamic allocation, no exceptions. All data stored in `std::array`.
- **Assert-guarded** -- runtime assertions protect every division path, index access, and precondition. Zero undefined behavior paths.
- **Right-handed coordinate system** -- column vectors with pre-multiplication (OpenGL/GLSL convention).
- **Row-major storage** by default, with configurable `MemoryOrder`.

## Headers

| Header | Purpose |
|--------|---------|
| `matrix.h` | Core `Matrix<T, ROWS, COLS, MEMORY_ORDER>` class |
| `matrix_operations.h` | `transpose`, `determinant`, `inverse`, `normalize`, `dot`, `norm` |
| `matrix_decomposition.h` | QR (Householder, Givens, Gram-Schmidt), Cholesky |
| `matrix_eigen_decomposition.h` | Francis QR eigenvalue/eigenvector algorithm |
| `vector.h` | `Vector<T, N>` (inherits Matrix Nx1) |
| `vector_operations.h` | `dot`, `cross`, `normalize`, `norm` |
| `point.h` | `Point<T, N>` -- affine point (w=1 semantics) |
| `point_operations.h` | `distance`, `distance_squared`, `signed_distance` |
| `quaternion.h` | `Quaternion<T>` -- Hamilton product, axis-angle, Euler conversion |
| `quaternion_operations.h` | `slerp`, `nlerp`, `log`, `exp`, `pow`, `axis`, `inverse` |
| `angle.h` | `Angle<T>`, `EulerAngles<T>`, `RotationConvention`, literals |
| `transform2.h` | 2D transforms: SO(2), SE(2), scale, rotation, translation |
| `transform3.h` | 3D transforms: SO(3), SE(3), Euler rotations, homogeneous |
| `interpolation.h` | `lerp` for scalars, matrices, vectors, points |
| `intersection.h` | Line-plane and line-line intersection (returns `std::optional`) |
| `frustum.h` | View frustum: perspective projection, plane extraction |
| `plane.h` | Hessian normal form plane |
| `barycentric.h` | Barycentric coordinates |
| `barycentric_operations.h` | `make_barycentric`, `contains` |
| `convex_polygon.h` | Fixed-capacity convex polygon container |
| `convex_polygon_operations.h` | Winding order, convexity check |
| `numeric.h` | `is_near_zero`, `is_near_equal`, `saturate`, epsilon utilities |
| `format.h` | `fmt::formatter` specializations |
| `ostream.h` | `operator<<` for `std::ostream` |

## Quick Start

```cpp
#include "math/vector.h"
#include "math/vector_operations.h"
#include "math/quaternion.h"
#include "math/quaternion_operations.h"
#include "math/transform3.h"

using namespace rtw::math;
using namespace rtw::math::angle_literals;

// Vectors and points
constexpr Vector3F direction{1.0F, 0.0F, 0.0F};
constexpr Point3F position{0.0F, 5.0F, 0.0F};
constexpr auto length = norm(direction);

// Quaternion rotation
const auto rotation = Quaternion<float>::from_axis_angle(
    Vector3F{0.0F, 1.0F, 0.0F}, 90.0_degF);
const auto rotated = rotation * direction;

// 3D transformation (SE(3))
const auto transform = transform3::make_se3(rotation, Vector3F{10.0F, 0.0F, 0.0F});
const auto inverse = transform3::inverse_se3(transform);

// Compile-time angle normalization
constexpr auto angle = normalize(0.5_radF + rtw::math::TAU<float>);
static_assert(angle.rad() > 0.49F);
```

## Type Aliases

### Vectors

```cpp
Vector2F, Vector2D, Vector2I, Vector2Q16, Vector2Q32  // 2D
Vector3F, Vector3D, Vector3I, Vector3Q16, Vector3Q32  // 3D
Vector4F, Vector4D, Vector4I, Vector4Q16, Vector4Q32  // 4D (homogeneous)
```

### Points

```cpp
Point2F, Point2D, Point2I, Point2Q16, Point2Q32  // 2D
Point3F, Point3D, Point3I, Point3Q16, Point3Q32  // 3D
Point4F, Point4D, Point4I, Point4Q16, Point4Q32  // 4D (homogeneous)
```

### Matrices

```cpp
Matrix2x2F, Matrix2x2D, Matrix2x2Q16, Matrix2x2Q32  // 2x2
Matrix3x3F, Matrix3x3D, Matrix3x3Q16, Matrix3x3Q32  // 3x3
Matrix4x4F, Matrix4x4D, Matrix4x4Q16, Matrix4x4Q32  // 4x4
```

### Angles

```cpp
AngleF, AngleD, AngleQ16, AngleQ32
EulerAnglesF, EulerAnglesD, EulerAnglesQ16, EulerAnglesQ32
```

## Point vs Vector Semantics

`Point` and `Vector` are distinct types with different algebraic behavior:

| Operation | Result |
|-----------|--------|
| `Point - Point` | `Vector` (displacement) |
| `Point + Vector` | `Point` (translated position) |
| `Vector + Vector` | `Vector` |
| `Point4::w` default | `1` (position in homogeneous space) |
| `Vector4::w` default | `0` (direction in homogeneous space) |

## Quaternion Operations

```cpp
// Construction
auto q = Quaternion<double>::from_axis_angle(axis, angle);
auto q = Quaternion<double>::from_euler_angles(roll, pitch, yaw);
auto q = Quaternion<double>::from_rotation_matrix(mat3x3);

// Operations
auto n    = normalize(q);
auto inv  = inverse(q);
auto conj = q.conjugate();
auto a    = axis(q);           // rotation axis (x-axis for identity)
auto l    = log(q);
auto e    = exp(q);
auto p    = pow(q, 0.5);       // square root of rotation

// Interpolation
auto r = slerp(q1, q2, t);    // constant angular velocity
auto r = nlerp(q1, q2, t);    // faster, normalized lerp

// Conversion
auto mat = q.to_rotation_matrix();
auto euler = q.to_euler_angles();
auto rotated_vector = q * vec3;  // rotate vector by quaternion
```

## Angle Literals

```cpp
using namespace rtw::math::angle_literals;

auto a = 90.0_degF;    // AngleF from degrees
auto b = 1.57_radD;    // AngleD from radians
auto c = 45.0_degQ16;  // AngleQ16 from degrees
```

## Transform Conventions

Transforms use **column vectors with pre-multiplication** (OpenGL/GLSL style):

```
v_world = M_model * v_local
v_clip  = M_projection * M_view * M_model * v_local
```

### 2D (SO(2) / SE(2))

```cpp
using namespace rtw::math::transform2;
auto R = make_rotation(angle);                    // 2x2 rotation
auto T = make_transform(scale, angle, offset);    // 3x3 homogeneous
auto inv = inverse_se2(T);
```

### 3D (SO(3) / SE(3))

```cpp
using namespace rtw::math::transform3;
auto Rx = make_rotation_x(angle);
auto R  = make_rotation(euler_angles, RotationConvention::YAW_PITCH_ROLL);
auto T  = make_se3(quaternion, translation);
auto inv = inverse_se3(T);
```

## Matrix Decompositions

```cpp
using namespace rtw::math::matrix_decomposition;

// QR decomposition (three algorithms)
auto [q, r] = qr::householder::decompose(A);
auto [q, r] = qr::givens::decompose(A);
auto [q, r] = qr::modified_gram_schmidt::decompose(A);

// Solve Ax = b
auto x = qr::householder::solve(A, b);

// Cholesky (symmetric positive-definite)
auto L = cholesky::decompose(A);  // returns std::optional
```

## Eigen Decomposition

```cpp
using namespace rtw::math::eigen_decomposition;

// Compute eigenvalues (QR algorithm with Francis double-shift)
auto result = eigenvalues(matrix, max_iterations, tolerance);
// result.values -- array of Complex<T>
// result.iterations -- number of QR iterations

// Compute eigenvectors
auto vecs = eigenvectors(matrix, result.values, tolerance);
```

## Frustum and Projection

```cpp
auto params = make_perspective_parameters(fov_y, aspect_ratio, near, far);
auto proj   = make_perspective_projection_matrix(params);
auto frustum = make_frustum(params);

// Or extract from existing projection matrix
auto frustum = extract_frustum(projection_matrix);
```

## Intersection Queries

All intersection functions return `std::optional` -- `std::nullopt` for parallel/degenerate cases:

```cpp
// Line-plane intersection (ND)
auto t = intersection_factor(plane_point, plane_normal, line_p0, line_p1);
auto pt = intersection(plane_point, plane_normal, line_p0, line_p1);

// 2D line-line intersection
auto t = intersection_factor(p0, p1, q0, q1);
auto pt = intersection(p0, p1, q0, q1);
```

## Barycentric Coordinates

```cpp
auto bary = make_barycentric(v0, v1, v2, point);
// bary.alpha(), bary.beta(), bary.gamma()

bool inside = contains(v0, v1, v2, point);
```

Degenerate triangles (collinear vertices) are caught by assertion.

## Safety Model

Every potentially-failing operation is guarded:

| Guard | Example |
|-------|---------|
| Division by zero | `assert(area != 0)` in barycentric |
| Index out of range | `assert(index < SIZE)` in `operator[]` |
| Zero-length normalize | `assert(n != 0)` in `normalize()` |
| Degenerate input | `assert(sin_theta != 0)` in slerp |
| Parallel lines | Returns `std::nullopt` from intersection |

Death tests verify every assert path fires correctly.

## Numeric Utilities

```cpp
// Epsilon-aware comparisons
is_near_zero(value, epsilon);
is_near_equal(a, b, epsilon);

// Type-appropriate defaults
default_near_zero_epsilon<float>();   // numeric_limits<float>::epsilon()
default_near_zero_epsilon<FixedPoint16>();  // 0 (exact arithmetic)

// Clamping
auto clamped = saturate(value);  // [0, 1]
```

## Testing

- 4,949 lines of tests across 16 test files
- 72 `static_assert` proofs (compile-time correctness)
- 32 death tests (assert coverage)
- Type-parameterized suites cover float, double, FixedPoint16, FixedPoint32

```sh
bazel test //math/...
```
