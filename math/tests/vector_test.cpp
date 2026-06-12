#include "math/format.h" // IWYU pragma: keep
#include "math/vector.h"
#include "math/vector_operations.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Vector, default_ctor)
{
  constexpr rtw::math::Vector2I V;
  EXPECT_THAT(V, ::testing::ElementsAre(0, 0));
}

TEST(Vector, init_list_ctor)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(Vector, coversion_ctor)
{
  constexpr rtw::math::Matrix<int, 2, 1> M{1, 2};
  constexpr rtw::math::Vector2I V{M};
  EXPECT_THAT(V, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_ctor_from_vector)
{
  constexpr rtw::math::Vector3I V1{1, 2, 3};
  constexpr rtw::math::Vector4I V2{V1};
  EXPECT_THAT(V2, ::testing::ElementsAre(1, 2, 3, 0));
}

TEST(Vector, copy_ctor)
{
  constexpr rtw::math::Vector2I V1{1, 2};
  constexpr rtw::math::Vector2I V2{V1};
  EXPECT_THAT(V2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_ctor)
{
  rtw::math::Vector2I v1{1, 2};
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  const rtw::math::Vector2I v2{std::move(v1)};
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, copy_assign)
{
  constexpr rtw::math::Vector2I V1{1, 2};
  rtw::math::Vector2I v2;
  v2 = V1;
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, move_assign)
{
  rtw::math::Vector2I v1{1, 2};
  rtw::math::Vector2I v2;
  // NOLINTNEXTLINE(performance-move-const-arg, hicpp-move-const-arg) -- this is a test
  v2 = std::move(v1);
  EXPECT_THAT(v2, ::testing::ElementsAre(1, 2));
}

TEST(Vector, as_matrix)
{
  constexpr rtw::math::Vector2I V{1, 2};
  constexpr auto M = V.as_matrix();
  EXPECT_EQ(M.rows(), 2);
  EXPECT_EQ(M.cols(), 1);
  EXPECT_THAT(M, ::testing::ElementsAre(1, 2));
}

TEST(Vector, operator_brackets)
{
  rtw::math::Vector2I v{1, 2};
  v[0] = 3;
  v[1] = 4;
  EXPECT_EQ(v[0], 3);
  EXPECT_EQ(v[1], 4);
}

TEST(Vector, operator_brackets_const)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_EQ(V[0], 1);
  EXPECT_EQ(V[1], 2);
}

TEST(Vector, operator_brackets_out_of_range)
{
  rtw::math::Vector2I v{1, 2};
  v[0] = 3;
  EXPECT_DEATH(v[2], "");
}

TEST(Vector, operator_brackets_const_out_of_range)
{
  constexpr rtw::math::Vector2I V{1, 2};
  EXPECT_DEATH(V[2], "");
}

TEST(Vector, operator_plus_equal)
{
  rtw::math::Vector2I v1{1, 2};
  constexpr rtw::math::Vector2I V2{3, 4};
  v1 += V2;
  EXPECT_THAT(v1, ::testing::ElementsAre(4, 6));
}

TEST(Vector, operator_minus_equal)
{
  rtw::math::Vector2I v1{1, 2};
  constexpr rtw::math::Vector2I V2{3, 4};
  v1 -= V2;
  EXPECT_THAT(v1, ::testing::ElementsAre(-2, -2));
}

TEST(Vector, operator_multiply_equal)
{
  rtw::math::Vector2I v{1, 2};
  v *= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(2, 4));
}

TEST(Vector, operator_divide_equal)
{
  rtw::math::Vector2I v{1, 2};
  v /= 2;
  EXPECT_THAT(v, ::testing::ElementsAre(0, 1));
}

TEST(Vector, operator_stream)
{
  constexpr rtw::math::Vector2I V{1, 2};
  std::stringstream ss;
  ss << V;
  constexpr auto EXPECTED = R"(Vector2[1 2])";
  EXPECT_EQ(ss.str(), EXPECTED);
}

TEST(Vector, accessors)
{
  {
    rtw::math::Vector3I v{1, 2, 3};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 3);

    v.x() = 4;
    v.y() = 5;
    v.z() = 6;
    EXPECT_EQ(v.x(), 4);
    EXPECT_EQ(v.y(), 5);
    EXPECT_EQ(v.z(), 6);
    EXPECT_EQ(v.w(), 6);

    v.w() = 9;
    EXPECT_EQ(v.z(), 9);
    EXPECT_EQ(v.w(), 9);
  }
  {
    rtw::math::Vector4I v{1, 2, 3, 4};
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
    EXPECT_EQ(v.z(), 3);
    EXPECT_EQ(v.w(), 4);

    v.x() = 5;
    v.y() = 6;
    v.z() = 7;
    v.w() = 8;
    EXPECT_EQ(v.x(), 5);
    EXPECT_EQ(v.y(), 6);
    EXPECT_EQ(v.z(), 7);
    EXPECT_EQ(v.w(), 8);

    v.w() = 11;
    EXPECT_EQ(v.w(), 11);
  }
}

TEST(Vector, accessors_const)
{
  {
    constexpr rtw::math::Vector3I V{1, 2, 3};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 3);
  }
  {
    constexpr rtw::math::Vector4I V{1, 2, 3, 4};
    EXPECT_EQ(V.x(), 1);
    EXPECT_EQ(V.y(), 2);
    EXPECT_EQ(V.z(), 3);
    EXPECT_EQ(V.w(), 4);
  }
}

TEST(Vector, norm2)
{
  constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
  constexpr auto NORM2 = rtw::math::norm2(V);
  EXPECT_FLOAT_EQ(NORM2, 14.0F);
}

TEST(Vector, norm)
{
  constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
  EXPECT_FLOAT_EQ(rtw::math::norm(V), std::sqrt(14.0F));
}

TEST(Vector, dot)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  constexpr auto DOT = rtw::math::dot(V1, V2);
  EXPECT_FLOAT_EQ(DOT, 32.0F);
}

TEST(Vector, cross2d)
{
  constexpr rtw::math::Vector2F V1{1.0F, 2.0F};
  constexpr rtw::math::Vector2F V2{3.0F, 4.0F};
  constexpr auto CROSS = rtw::math::cross(V1, V2);
  EXPECT_FLOAT_EQ(CROSS, -2.0F);
}

TEST(Vector, cross3d)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  constexpr auto V3 = rtw::math::cross(V1, V2);
  EXPECT_THAT(V3, ::testing::ElementsAre(-3.0F, 6.0F, -3.0F));
}

TEST(Vector, operator_plus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  const auto v3 = V1 + V2;
  EXPECT_THAT(v3, ::testing::ElementsAre(5.0F, 7.0F, 9.0F));
}

TEST(Vector, operator_minus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{4.0F, 5.0F, 6.0F};
  const auto v3 = V1 - V2;
  EXPECT_THAT(v3, ::testing::ElementsAre(-3.0F, -3.0F, -3.0F));
}

TEST(Vector, operator_multiply)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = V1 * 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
  const auto v3 = 2.0F * V1;
  EXPECT_THAT(v3, ::testing::ElementsAre(2.0F, 4.0F, 6.0F));
}

TEST(Vector, operator_divide)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = V1 / 2.0F;
  EXPECT_THAT(v2, ::testing::ElementsAre(0.5F, 1.0F, 1.5F));
}

TEST(Vector, operator_equal)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F V2{1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(V1 == V2);
  EXPECT_FALSE(V1 != V2);
}

TEST(Vector, operator_unary_minus)
{
  constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
  const auto v2 = -V1;
  EXPECT_THAT(v2, ::testing::ElementsAre(-1.0F, -2.0F, -3.0F));
}

TEST(Vector, static_properties)
{
  // Default construction is zero
  static_assert(rtw::math::Vector3I{} == rtw::math::Vector3I{0, 0, 0});

  // Element access
  constexpr rtw::math::Vector3I V{1, 2, 3};
  static_assert(V.x() == 1);
  static_assert(V.y() == 2);
  static_assert(V.z() == 3);
  static_assert(V[0] == 1);
  static_assert(V[1] == 2);
  static_assert(V[2] == 3);

  // Dot product
  static_assert(rtw::math::dot(V, rtw::math::Vector3I{4, 5, 6}) == 32);

  // Cross product (directly constructs result, no UNINITIALIZED)
  static_assert(rtw::math::cross(rtw::math::Vector3I{1, 0, 0}, rtw::math::Vector3I{0, 1, 0})
                == rtw::math::Vector3I{0, 0, 1});

  // Norm squared (via dot)
  static_assert(rtw::math::norm2(V) == 14);

  // Copy construction preserves values
  constexpr rtw::math::Vector3I V2{V};
  static_assert(V2 == V);

  // Widening copy (3D -> 4D pads with zero)
  constexpr rtw::math::Vector4I V4{V};
  static_assert(V4[0] == 1);
  static_assert(V4[1] == 2);
  static_assert(V4[2] == 3);
  static_assert(V4[3] == 0);
}

TEST(Vector, normalize)
{
  {
    constexpr rtw::math::Vector3F V1{1.0F, 2.0F, 3.0F};
    const auto v2 = rtw::math::normalize(V1);
    EXPECT_THAT(v2, ::testing::ElementsAre(1.0F / std::sqrt(14.0F), 2.0F / std::sqrt(14.0F), 3.0F / std::sqrt(14.0F)));
  }
  {
    EXPECT_DEATH(rtw::math::normalize(rtw::math::Vector3F{0.0F, 0.0F, 0.0F}), "");
  }
}

TEST(Vector, swizzle)
{
  {
    constexpr rtw::math::Vector4F V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xy();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F));
  }
  {
    constexpr rtw::math::Vector4F V1{1.0F, 2.0F, 3.0F, 4.0F};
    constexpr auto V2 = V1.xyz();
    EXPECT_THAT(V2, ::testing::ElementsAre(1.0F, 2.0F, 3.0F));
  }
}

TEST(Vector, scalar_projection)
{
  {
    constexpr rtw::math::Vector2F V{3.0F, 4.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 0.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 3.0F);
  }
  {
    constexpr rtw::math::Vector2F V{3.0F, 4.0F};
    constexpr rtw::math::Vector2F ONTO{0.0F, 1.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 4.0F);
  }
  {
    constexpr rtw::math::Vector2F V{1.0F, 1.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 0.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 1.0F);
  }
  {
    constexpr rtw::math::Vector2F V{6.0F, 0.0F};
    constexpr rtw::math::Vector2F ONTO{2.0F, 0.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 6.0F);
  }
  {
    constexpr rtw::math::Vector2F V{1.0F, 0.0F};
    constexpr rtw::math::Vector2F ONTO{-1.0F, 0.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), -1.0F);
  }
  {
    constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
    constexpr rtw::math::Vector3F ONTO{1.0F, 0.0F, 0.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 1.0F);
  }
  {
    constexpr rtw::math::Vector2F V{3.0F, 3.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 1.0F};
    EXPECT_FLOAT_EQ(rtw::math::scalar_projection(V, ONTO), 3.0F * std::sqrt(2.0F));
  }
  {
    constexpr rtw::math::Vector2F V{1.0F, 1.0F};
    constexpr rtw::math::Vector2F ONTO{0.0F, 0.0F};
    EXPECT_DEATH(rtw::math::scalar_projection(V, ONTO), "");
  }
}

TEST(Vector, vector_projection)
{
  {
    constexpr rtw::math::Vector2F V{3.0F, 4.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 0.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(3.0F, 0.0F));
  }
  {
    constexpr rtw::math::Vector2F V{3.0F, 4.0F};
    constexpr rtw::math::Vector2F ONTO{0.0F, 1.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(0.0F, 4.0F));
  }
  {
    constexpr rtw::math::Vector2F V{6.0F, 8.0F};
    constexpr rtw::math::Vector2F ONTO{3.0F, 0.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(6.0F, 0.0F));
  }
  {
    constexpr rtw::math::Vector2F V{4.0F, 0.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 1.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(2.0F, 2.0F));
  }
  {
    constexpr rtw::math::Vector2F V{3.0F, 4.0F};
    const auto result = rtw::math::vector_projection(V, V);
    EXPECT_FLOAT_EQ(result.x(), 3.0F);
    EXPECT_FLOAT_EQ(result.y(), 4.0F);
  }
  {
    constexpr rtw::math::Vector3F V{1.0F, 2.0F, 3.0F};
    constexpr rtw::math::Vector3F ONTO{1.0F, 0.0F, 0.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(1.0F, 0.0F, 0.0F));
  }
  {
    constexpr rtw::math::Vector2F V{0.0F, 1.0F};
    constexpr rtw::math::Vector2F ONTO{1.0F, 0.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(0.0F, 0.0F));
  }
  {
    constexpr rtw::math::Vector2F V{2.0F, 0.0F};
    constexpr rtw::math::Vector2F ONTO{-1.0F, 0.0F};
    const auto result = rtw::math::vector_projection(V, ONTO);
    EXPECT_THAT(result, ::testing::ElementsAre(2.0F, 0.0F));
  }
  {
    constexpr rtw::math::Vector2F V{1.0F, 1.0F};
    constexpr rtw::math::Vector2F ONTO{0.0F, 0.0F};
    EXPECT_DEATH(rtw::math::vector_projection(V, ONTO), "");
  }
}

TEST(Vector, hadamard)
{
  constexpr rtw::math::Vector3F A{1.0F, 2.0F, 3.0F};
  constexpr rtw::math::Vector3F B{4.0F, 5.0F, 6.0F};
  EXPECT_THAT(rtw::math::hadamard(A, B), ::testing::ElementsAre(4.0F, 10.0F, 18.0F));
}

TEST(Vector, min)
{
  constexpr rtw::math::Vector3F A{1.0F, 5.0F, 3.0F};
  constexpr rtw::math::Vector3F B{4.0F, 2.0F, 3.0F};
  EXPECT_THAT(rtw::math::min(A, B), ::testing::ElementsAre(1.0F, 2.0F, 3.0F));
  EXPECT_THAT(rtw::math::min(A, 3.0F), ::testing::ElementsAre(1.0F, 3.0F, 3.0F));
}

TEST(Vector, max)
{
  constexpr rtw::math::Vector3F A{1.0F, 5.0F, 3.0F};
  constexpr rtw::math::Vector3F B{4.0F, 2.0F, 3.0F};
  EXPECT_THAT(rtw::math::max(A, B), ::testing::ElementsAre(4.0F, 5.0F, 3.0F));
  EXPECT_THAT(rtw::math::max(A, 3.0F), ::testing::ElementsAre(3.0F, 5.0F, 3.0F));
}

TEST(Vector, clamp)
{
  constexpr rtw::math::Vector3F V{-1.0F, 0.5F, 2.0F};
  constexpr rtw::math::Vector3F LO{0.0F, 0.0F, 0.0F};
  constexpr rtw::math::Vector3F HI{1.0F, 1.0F, 1.0F};
  EXPECT_THAT(rtw::math::clamp(V, LO, HI), ::testing::ElementsAre(0.0F, 0.5F, 1.0F));
  EXPECT_THAT(rtw::math::clamp(V, 0.0F, 1.0F), ::testing::ElementsAre(0.0F, 0.5F, 1.0F));
}

TEST(Vector, operations_integer)
{
  using namespace rtw::math;

  // Hadamard (element-wise) product.
  EXPECT_THAT(hadamard(Vector3I{1, 2, 3}, Vector3I{4, 5, 6}), ::testing::ElementsAre(4, 10, 18));

  // Component-wise min/max.
  EXPECT_THAT(min(Vector3I{1, 5, 3}, Vector3I{4, 2, 3}), ::testing::ElementsAre(1, 2, 3));
  EXPECT_THAT(max(Vector3I{1, 5, 3}, Vector3I{4, 2, 3}), ::testing::ElementsAre(4, 5, 3));

  // Scalar-bound min/max.
  EXPECT_THAT(min(Vector3I{1, 5, 3}, 3), ::testing::ElementsAre(1, 3, 3));
  EXPECT_THAT(max(Vector3I{1, 5, 3}, 3), ::testing::ElementsAre(3, 5, 3));

  // Clamp with per-component and scalar bounds.
  EXPECT_THAT(clamp(Vector3I{-1, 5, 20}, Vector3I{0, 0, 0}, Vector3I{10, 10, 10}), ::testing::ElementsAre(0, 5, 10));
  EXPECT_THAT(clamp(Vector3I{-1, 5, 20}, 0, 10), ::testing::ElementsAre(0, 5, 10));
}

template <typename T>
class SignedFixedPointVectorOpsTest : public ::testing::Test
{};
using SignedFixedPointVectorTypes =
    ::testing::Types<rtw::multiprecision::FixedPoint8, rtw::multiprecision::FixedPoint16,
                     rtw::multiprecision::FixedPoint32>;
TYPED_TEST_SUITE(SignedFixedPointVectorOpsTest, SignedFixedPointVectorTypes, );

TYPED_TEST(SignedFixedPointVectorOpsTest, hadamard)
{
  using namespace rtw::math;
  using Vec = Vector3<TypeParam>;
  const auto result = hadamard(Vec{TypeParam(2.0), TypeParam(3.0), TypeParam(-4.0)},
                               Vec{TypeParam(4.0), TypeParam(5.0), TypeParam(2.0)});
  EXPECT_NEAR(static_cast<double>(result[0]), 8.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(result[1]), 15.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(result[2]), -8.0, TypeParam::RESOLUTION);
}

TYPED_TEST(SignedFixedPointVectorOpsTest, min_max)
{
  using namespace rtw::math;
  using Vec = Vector3<TypeParam>;
  const Vec a{TypeParam(1.0), TypeParam(5.0), TypeParam(-3.0)};
  const Vec b{TypeParam(4.0), TypeParam(2.0), TypeParam(3.0)};

  const auto mn = min(a, b);
  EXPECT_NEAR(static_cast<double>(mn[0]), 1.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mn[1]), 2.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mn[2]), -3.0, TypeParam::RESOLUTION);

  const auto mx = max(a, b);
  EXPECT_NEAR(static_cast<double>(mx[0]), 4.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mx[1]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mx[2]), 3.0, TypeParam::RESOLUTION);

  const auto mns = min(a, TypeParam(2.0));
  EXPECT_NEAR(static_cast<double>(mns[1]), 2.0, TypeParam::RESOLUTION);
  const auto mxs = max(a, TypeParam(2.0));
  EXPECT_NEAR(static_cast<double>(mxs[2]), 2.0, TypeParam::RESOLUTION);
}

TYPED_TEST(SignedFixedPointVectorOpsTest, clamp)
{
  using namespace rtw::math;
  using Vec = Vector3<TypeParam>;
  const Vec v{TypeParam(-1.0), TypeParam(5.0), TypeParam(20.0)};
  const Vec lo{TypeParam(0.0), TypeParam(0.0), TypeParam(0.0)};
  const Vec hi{TypeParam(10.0), TypeParam(10.0), TypeParam(10.0)};

  const auto c = clamp(v, lo, hi);
  EXPECT_NEAR(static_cast<double>(c[0]), 0.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(c[1]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(c[2]), 10.0, TypeParam::RESOLUTION);

  const auto cs = clamp(v, TypeParam(0.0), TypeParam(10.0));
  EXPECT_NEAR(static_cast<double>(cs[0]), 0.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(cs[1]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(cs[2]), 10.0, TypeParam::RESOLUTION);
}

template <typename T>
class UnsignedFixedPointVectorOpsTest : public ::testing::Test
{};
using UnsignedFixedPointVectorTypes =
    ::testing::Types<rtw::multiprecision::FixedPoint8U, rtw::multiprecision::FixedPoint16U,
                     rtw::multiprecision::FixedPoint32U>;
TYPED_TEST_SUITE(UnsignedFixedPointVectorOpsTest, UnsignedFixedPointVectorTypes, );

TYPED_TEST(UnsignedFixedPointVectorOpsTest, min_max_clamp)
{
  using namespace rtw::math;
  using Vec = Vector3<TypeParam>;
  const Vec a{TypeParam(1.0), TypeParam(5.0), TypeParam(3.0)};
  const Vec b{TypeParam(4.0), TypeParam(2.0), TypeParam(3.0)};

  const auto mn = min(a, b);
  EXPECT_NEAR(static_cast<double>(mn[0]), 1.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mn[1]), 2.0, TypeParam::RESOLUTION);

  const auto mx = max(a, b);
  EXPECT_NEAR(static_cast<double>(mx[0]), 4.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(mx[1]), 5.0, TypeParam::RESOLUTION);

  const auto c =
      clamp(Vec{TypeParam(1.0), TypeParam(5.0), TypeParam(20.0)}, Vec{TypeParam(0.0), TypeParam(0.0), TypeParam(0.0)},
            Vec{TypeParam(10.0), TypeParam(10.0), TypeParam(10.0)});
  EXPECT_NEAR(static_cast<double>(c[0]), 1.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(c[1]), 5.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(c[2]), 10.0, TypeParam::RESOLUTION);
}

TYPED_TEST(UnsignedFixedPointVectorOpsTest, hadamard)
{
  using namespace rtw::math;
  using Vec = Vector2<TypeParam>;

  const auto h = hadamard(Vec{TypeParam(2.0), TypeParam(3.0)}, Vec{TypeParam(4.0), TypeParam(5.0)});
  EXPECT_NEAR(static_cast<double>(h[0]), 8.0, TypeParam::RESOLUTION);
  EXPECT_NEAR(static_cast<double>(h[1]), 15.0, TypeParam::RESOLUTION);
}
