#include "math/angle.h"

#include <gtest/gtest.h>

constexpr auto epsilon = 1e-7;

TEST(Angle, rad)
{
  using namespace rtw::math::angle_literals;
  const auto angle = 0.5_rad;
  EXPECT_DOUBLE_EQ(angle.rad(), 0.5);
}

TEST(Angle, deg)
{
  using namespace rtw::math::angle_literals;
  const auto angle = 0.5_deg;
  EXPECT_DOUBLE_EQ(angle.deg(), 0.5);
}

TEST(Angle, normalize)
{
  using namespace rtw::math::angle_literals;
  EXPECT_NEAR(rtw::math::normalize(0.5_rad).rad(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::normalize(0.5_deg).deg(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::normalize(0.5_rad + rtw::math::tao<double>).rad(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::normalize(0.5_deg + 360.0_deg).deg(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::normalize(0.5_rad - rtw::math::tao<double>).rad(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::normalize(0.5_deg - 360.0_deg).deg(), 0.5, epsilon);
}

TEST(Angle, distance)
{
  using namespace rtw::math::angle_literals;
  EXPECT_NEAR(rtw::math::distance(0.5_rad, 0.5_rad), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_deg, 0.5_deg), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_rad, 0.5_rad + rtw::math::tao<double>), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_deg, 0.5_deg + 360.0_deg), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_rad, 0.5_rad - rtw::math::tao<double>), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_deg, 0.5_deg - 360.0_deg), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_rad, 0.5_rad + rtw::math::pi_2<double>), rtw::math::pi_2<double>, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_deg, 0.5_deg + 90.0_deg), 90.0_deg, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_rad, 0.5_rad - rtw::math::pi_2<double>), -rtw::math::pi_2<double>, epsilon);
  EXPECT_NEAR(rtw::math::distance(0.5_deg, 0.5_deg - 90.0_deg), -90.0_deg, epsilon);
}

TEST(Angle, interpolate)
{
  using namespace rtw::math::angle_literals;
  EXPECT_NEAR(rtw::math::interpolate(0.0_rad, 1.0_rad, 0.0).rad(), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_deg, 1.0_deg, 0.0).deg(), 0.0, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_rad, 1.0_rad, 1.0).rad(), 1.0, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_deg, 1.0_deg, 1.0).deg(), 1.0, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_rad, 1.0_rad, 0.5).rad(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_deg, 1.0_deg, 0.5).deg(), 0.5, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_rad, 1.0_rad, 0.25).rad(), 0.25, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_deg, 1.0_deg, 0.25).deg(), 0.25, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_rad, 1.0_rad, 0.75).rad(), 0.75, epsilon);
  EXPECT_NEAR(rtw::math::interpolate(0.0_deg, 1.0_deg, 0.75).deg(), 0.75, epsilon);
}
