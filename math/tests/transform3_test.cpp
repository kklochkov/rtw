#include "math/transform3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Transform3, make_scale)
{
  constexpr auto MATRIX = rtw::math::transform3::make_scale(1.0, 2.0, 3.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 1), 2.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 2), 3.0);
}

TEST(Transform3, make_rotation_x)
{
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_x(ANGLE);
  ASSERT_DOUBLE_EQ(matrix(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 2), -std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 2), std::cos(ANGLE.rad()));
}

TEST(Transform3, make_rotation_y)
{
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_y(ANGLE);
  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 0), -std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), std::cos(ANGLE.rad()));
}

TEST(Transform3, make_rotation_z)
{
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_z(ANGLE);
  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), -std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), 1.0);
}

TEST(Transform3, make_translation)
{
  constexpr auto TRANSLATION = rtw::math::Vector3d{1.0, 2.0, 3.0};
  constexpr auto MATRIX = rtw::math::transform3::make_translation(TRANSLATION);

  ASSERT_DOUBLE_EQ(MATRIX(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 2), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 3), TRANSLATION.x());
  ASSERT_DOUBLE_EQ(MATRIX(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 2), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 3), TRANSLATION.y());
  ASSERT_DOUBLE_EQ(MATRIX(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 2), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 3), TRANSLATION.z());
  ASSERT_DOUBLE_EQ(MATRIX(3, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(3, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(3, 2), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(3, 3), 1.0);
}

TEST(Transform3, make_transform)
{
  using namespace rtw::math::angle_literals;
  constexpr auto ROLL = 90.0_deg;
  constexpr auto PITCH = 90.0_deg;
  constexpr auto YAW = 90.0_deg;
  constexpr auto TRANSLATION = rtw::math::Vector3d{1.0, 2.0, 3.0};
  const auto se3 = rtw::math::transform3::make_transform(ROLL, PITCH, YAW, TRANSLATION);
  const auto expected_rotation = rtw::math::transform3::make_rotation_z(YAW)
                               * rtw::math::transform3::make_rotation_y(PITCH)
                               * rtw::math::transform3::make_rotation_x(ROLL);
  ASSERT_EQ(rtw::math::transform3::rotation(se3), expected_rotation);
  ASSERT_EQ(rtw::math::transform3::translation(se3), TRANSLATION);
}

TEST(Transform3, inverse_rotation)
{
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = rtw::math::transform3::make_rotation_z(ANGLE);
  const auto inverse = rtw::math::transform3::inverse_rotation(matrix);

  {
    ASSERT_DOUBLE_EQ(inverse(0, 0), std::cos(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 1), std::sin(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 2), 0.0);
    ASSERT_DOUBLE_EQ(inverse(1, 0), -std::sin(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 1), std::cos(ANGLE.rad()));
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
  constexpr auto ROLL = 90.0_deg;
  constexpr auto PITCH = 90.0_deg;
  constexpr auto YAW = 90.0_deg;
  constexpr auto TRANSLATION = rtw::math::Vector3d{1.0, 2.0, 3.0};
  const auto se3 = rtw::math::transform3::make_transform(ROLL, PITCH, YAW, TRANSLATION);
  const auto inverse = rtw::math::transform3::inverse_transform(se3);

  {
    const auto expected_rotation = rtw::math::transform3::make_rotation_x(-ROLL)
                                 * rtw::math::transform3::make_rotation_y(-PITCH)
                                 * rtw::math::transform3::make_rotation_z(-YAW);
    ASSERT_EQ(rtw::math::transform3::rotation(inverse), expected_rotation);
    ASSERT_EQ(rtw::math::transform3::translation(inverse), -expected_rotation * TRANSLATION);
  }
}
