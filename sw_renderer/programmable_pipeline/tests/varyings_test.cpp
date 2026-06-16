#include "sw_renderer/programmable_pipeline/varyings.h"

#include "sw_renderer/programmable_pipeline/register_file.h"

#include "math/vector.h"

#include <gtest/gtest.h>

namespace
{

namespace sw = rtw::sw_renderer;

using single_precision = float;
using Vec3 = rtw::math::Vector3<single_precision>;
using Vec4 = rtw::math::Vector4<single_precision>;

struct TestVaryings : sw::VaryingsBase<TestVaryings, single_precision, 2>
{
  Vec3 world_pos() const { return regs[0].xyz(); }
  void set_world_pos(const Vec3& position)
  {
    regs[0] = Vec4{position.x(), position.y(), position.z(), single_precision{0}};
  }

  single_precision intensity() const { return regs[1].x(); }
  void set_intensity(const single_precision value)
  {
    regs[1] = Vec4{value, single_precision{0}, single_precision{0}, single_precision{0}};
  }
};

TEST(Varyings, default_ctor_zeroes_slots)
{
  const TestVaryings v;
  EXPECT_EQ(v.world_pos(), Vec3{});
  EXPECT_EQ(v.intensity(), 0.0F);
}

TEST(Varyings, accessors_map_to_slots)
{
  TestVaryings v;
  v.set_world_pos(Vec3{1.0F, 2.0F, 3.0F});
  v.set_intensity(4.0F);

  EXPECT_EQ(v.world_pos(), (Vec3{1.0F, 2.0F, 3.0F}));
  EXPECT_EQ(v.intensity(), 4.0F);

  // The accessors are a typed view over the raw register-file slots.
  EXPECT_EQ(v.regs[0].x(), 1.0F);
  EXPECT_EQ(v.regs[0].y(), 2.0F);
  EXPECT_EQ(v.regs[0].z(), 3.0F);
  EXPECT_EQ(v.regs[1].x(), 4.0F);
}

TEST(Varyings, addition_forwards_to_register_file)
{
  TestVaryings a;
  a.set_world_pos(Vec3{1.0F, 2.0F, 3.0F});
  a.set_intensity(1.0F);
  TestVaryings b;
  b.set_world_pos(Vec3{10.0F, 20.0F, 30.0F});
  b.set_intensity(2.0F);

  const auto c = a + b;
  EXPECT_EQ(c.world_pos(), (Vec3{11.0F, 22.0F, 33.0F}));
  EXPECT_EQ(c.intensity(), 3.0F);
  EXPECT_TRUE(c.regs == (a.regs + b.regs));
}

TEST(Varyings, scalar_multiplication_forwards_to_register_file)
{
  TestVaryings a;
  a.set_world_pos(Vec3{1.0F, 2.0F, 3.0F});
  a.set_intensity(4.0F);

  const auto b = a * 2.0F;
  const auto c = 2.0F * a; // commutative overload
  EXPECT_EQ(b.world_pos(), (Vec3{2.0F, 4.0F, 6.0F}));
  EXPECT_EQ(b.intensity(), 8.0F);
  EXPECT_TRUE(b.regs == c.regs);
}

TEST(Varyings, lerp_derived_from_concept_operators)
{
  TestVaryings a;
  a.set_world_pos(Vec3{0.0F, 0.0F, 0.0F});
  a.set_intensity(0.0F);
  TestVaryings b;
  b.set_world_pos(Vec3{10.0F, 20.0F, 30.0F});
  b.set_intensity(4.0F);

  // a * (1 - t) + b * t, expressed purely via the Varyings concept operators.
  const single_precision t{0.5F};
  const auto mid = a * (1.0F - t) + b * t;
  EXPECT_EQ(mid.world_pos(), (Vec3{5.0F, 10.0F, 15.0F}));
  EXPECT_EQ(mid.intensity(), 2.0F);
}

} // namespace
