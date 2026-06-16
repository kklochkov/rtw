#include "sw_renderer/programmable_pipeline/register_file.h"

#include "math/vector.h"
#include "multiprecision/fixed_point.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace
{

namespace sw = rtw::sw_renderer;
namespace mp = rtw::multiprecision;

template <typename T>
using Vec4 = rtw::math::Vector4<T>;

template <typename T>
void expect_slot_eq(const Vec4<T>& slot, const T x, const T y, const T z, const T w)
{
  EXPECT_EQ(slot.x(), x);
  EXPECT_EQ(slot.y(), y);
  EXPECT_EQ(slot.z(), z);
  EXPECT_EQ(slot.w(), w);
}

template <typename T>
class RegisterFileTest : public ::testing::Test
{};

using ScalarTypes = ::testing::Types<float, mp::FixedPoint16>;
TYPED_TEST_SUITE(RegisterFileTest, ScalarTypes, );

TYPED_TEST(RegisterFileTest, default_ctor_zeroes_all_slots)
{
  using T = TypeParam;
  const sw::RegisterFile<T, 3> rf;
  EXPECT_EQ(rf.size(), 3);
  for (std::uint16_t i = 0U; i < rf.size(); ++i)
  {
    expect_slot_eq<T>(rf[i], T{0}, T{0}, T{0}, T{0});
  }
}

TYPED_TEST(RegisterFileTest, slot_read_write)
{
  using T = TypeParam;
  sw::RegisterFile<T, 2> rf;
  rf[0] = Vec4<T>{T{1}, T{2}, T{3}, T{4}};
  rf[1] = Vec4<T>{T{5}, T{6}, T{7}, T{8}};
  expect_slot_eq<T>(rf[0], T{1}, T{2}, T{3}, T{4});
  expect_slot_eq<T>(rf[1], T{5}, T{6}, T{7}, T{8});
}

TYPED_TEST(RegisterFileTest, addition)
{
  using T = TypeParam;
  sw::RegisterFile<T, 2> a;
  a[0] = Vec4<T>{T{1}, T{2}, T{3}, T{4}};
  a[1] = Vec4<T>{T{5}, T{6}, T{7}, T{8}};
  sw::RegisterFile<T, 2> b;
  b[0] = Vec4<T>{T{1}, T{1}, T{1}, T{1}};
  b[1] = Vec4<T>{T{2}, T{2}, T{2}, T{2}};

  const auto c = a + b;
  expect_slot_eq<T>(c[0], T{2}, T{3}, T{4}, T{5});
  expect_slot_eq<T>(c[1], T{7}, T{8}, T{9}, T{10});

  sw::RegisterFile<T, 2> d = a;
  d += b;
  EXPECT_TRUE(d == c);
}

TYPED_TEST(RegisterFileTest, scalar_multiplication)
{
  using T = TypeParam;
  sw::RegisterFile<T, 2> a;
  a[0] = Vec4<T>{T{1}, T{2}, T{3}, T{4}};
  a[1] = Vec4<T>{T{5}, T{6}, T{7}, T{8}};

  const auto b = a * T{2};
  const auto c = T{2} * a; // commutative overload
  expect_slot_eq<T>(b[0], T{2}, T{4}, T{6}, T{8});
  expect_slot_eq<T>(b[1], T{10}, T{12}, T{14}, T{16});
  EXPECT_TRUE(b == c);

  sw::RegisterFile<T, 2> d = a;
  d *= T{2};
  EXPECT_TRUE(d == b);
}

TYPED_TEST(RegisterFileTest, equality)
{
  using T = TypeParam;
  sw::RegisterFile<T, 2> a;
  a[0] = Vec4<T>{T{1}, T{2}, T{3}, T{4}};
  sw::RegisterFile<T, 2> b = a;
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  b[1] = Vec4<T>{T{1}, T{0}, T{0}, T{0}};
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}

TYPED_TEST(RegisterFileTest, lerp_endpoints_and_midpoint)
{
  using T = TypeParam;
  sw::RegisterFile<T, 2> a;
  a[0] = Vec4<T>{T{0}, T{0}, T{0}, T{0}};
  a[1] = Vec4<T>{T{2}, T{4}, T{6}, T{8}};
  sw::RegisterFile<T, 2> b;
  b[0] = Vec4<T>{T{10}, T{20}, T{30}, T{40}};
  b[1] = Vec4<T>{T{4}, T{8}, T{12}, T{16}};

  EXPECT_TRUE(sw::lerp(a, b, T{0}) == a);
  EXPECT_TRUE(sw::lerp(a, b, T{1}) == b);

  const auto mid = sw::lerp(a, b, T{0.5});
  expect_slot_eq<T>(mid[0], T{5}, T{10}, T{15}, T{20});
  expect_slot_eq<T>(mid[1], T{3}, T{6}, T{9}, T{12});
}

TYPED_TEST(RegisterFileTest, range_iteration)
{
  using T = TypeParam;
  sw::RegisterFile<T, 3> rf;
  rf[0] = Vec4<T>{T{1}, T{0}, T{0}, T{0}};
  rf[1] = Vec4<T>{T{2}, T{0}, T{0}, T{0}};
  rf[2] = Vec4<T>{T{3}, T{0}, T{0}, T{0}};

  T sum{0};
  for (const auto& slot : rf)
  {
    sum += slot.x();
  }
  EXPECT_EQ(sum, T{6});
}

// --- constexpr usability: the substrate avoids the UNINITIALIZED ctor, so it is genuinely usable
// in constant expressions (unlike the matrix/vector free ops). --------------------------------

constexpr sw::RegisterFile<float, 2> make_rf(const Vec4<float>& slot0, const Vec4<float>& slot1) noexcept
{
  sw::RegisterFile<float, 2> rf;
  rf[0] = slot0;
  rf[1] = slot1;
  return rf;
}

constexpr auto A = make_rf(Vec4<float>{0.0F, 0.0F, 0.0F, 0.0F}, Vec4<float>{2.0F, 4.0F, 6.0F, 8.0F});
constexpr auto B = make_rf(Vec4<float>{10.0F, 20.0F, 30.0F, 40.0F}, Vec4<float>{4.0F, 8.0F, 12.0F, 16.0F});

static_assert(make_rf(Vec4<float>{}, Vec4<float>{})[0] == Vec4<float>{});
static_assert((A + B)[0] == Vec4<float>{10.0F, 20.0F, 30.0F, 40.0F});
static_assert((A * 2.0F)[1] == Vec4<float>{4.0F, 8.0F, 12.0F, 16.0F});

} // namespace
