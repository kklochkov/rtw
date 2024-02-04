#include "math/transform3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Transform3, make_scale)
{
  constexpr auto matrix = rtw::math::transform3::make_scale(1.0, 2.0, 3.0);
  ASSERT_DOUBLE_EQ(matrix(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), 2.0);
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), 3.0);
}

TEST(Transform3, make_rotation_x)
{
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_x(angle);
  ASSERT_DOUBLE_EQ(matrix(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 2), -std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 2), std::cos(angle.rad()));
}

TEST(Transform3, make_rotation_y)
{
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_y(angle);
  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 0), -std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), std::cos(angle.rad()));
}

TEST(Transform3, make_rotation_z)
{
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_z(angle);
  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), -std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), 1.0);
}

TEST(Transform3, make_translation)
{
  constexpr auto translation = rtw::math::Vector3d{1.0, 2.0, 3.0};
  constexpr auto matrix = rtw::math::transform3::make_translation(translation);

  ASSERT_DOUBLE_EQ(matrix(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 3), translation.x());
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 3), translation.y());
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), 1.0);
  ASSERT_DOUBLE_EQ(matrix(2, 3), translation.z());
  ASSERT_DOUBLE_EQ(matrix(3, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(3, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(3, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(3, 3), 1.0);
}

TEST(Transform3, make_transform)
{
  using namespace rtw::math::angle_literals;
  constexpr auto roll = 90.0_deg;
  constexpr auto pitch = 90.0_deg;
  constexpr auto yaw = 90.0_deg;
  constexpr auto translation = rtw::math::Vector3d{1.0, 2.0, 3.0};
  const auto se3 = rtw::math::transform3::make_transform(roll, pitch, yaw, translation);
  const auto expected_rotation = rtw::math::transform3::make_rotation_z(yaw) *
                                 rtw::math::transform3::make_rotation_y(pitch) *
                                 rtw::math::transform3::make_rotation_x(roll);
  ASSERT_EQ(rtw::math::transform3::rotation(se3), expected_rotation);
  ASSERT_EQ(rtw::math::transform3::translation(se3), translation);
}

TEST(Transform3, inverse_rotation)
{
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_z(angle);
  const auto inverse = rtw::math::transform3::inverse_rotation(matrix);

  {
    ASSERT_DOUBLE_EQ(inverse(0, 0), std::cos(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 1), std::sin(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 2), 0.0);
    ASSERT_DOUBLE_EQ(inverse(1, 0), -std::sin(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 1), std::cos(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 2), 0.0);
    ASSERT_DOUBLE_EQ(inverse(2, 0), 0.0);
    ASSERT_DOUBLE_EQ(inverse(2, 1), 0.0);
    ASSERT_DOUBLE_EQ(inverse(2, 2), 1.0);
  }

  {
    const auto inverse_matrix = rtw::math::inverse(matrix);
    ASSERT_EQ(inverse, inverse_matrix);
  }
}

TEST(Transform3, inverse_transform)
{
  using namespace rtw::math::angle_literals;
  constexpr auto roll = 90.0_deg;
  constexpr auto pitch = 90.0_deg;
  constexpr auto yaw = 90.0_deg;
  constexpr auto translation = rtw::math::Vector3d{1.0, 2.0, 3.0};
  const auto se3 = rtw::math::transform3::make_transform(roll, pitch, yaw, translation);
  const auto inverse = rtw::math::transform3::inverse_transform(se3);

  {
    const auto expected_rotation = rtw::math::transform3::make_rotation_x(-roll) *
                                   rtw::math::transform3::make_rotation_y(-pitch) *
                                   rtw::math::transform3::make_rotation_z(-yaw);
    ASSERT_EQ(rtw::math::transform3::rotation(inverse), expected_rotation);
    ASSERT_EQ(rtw::math::transform3::translation(inverse), -expected_rotation * translation);
  }
}
