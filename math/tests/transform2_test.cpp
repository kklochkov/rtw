#include "math/transform2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Transform2, make_scale)
{
  using namespace rtw::math::transform2;
  constexpr auto SCALE = rtw::math::Vector2d{1.0, 2.0};
  constexpr auto MATRIX = make_scale(SCALE);

  ASSERT_DOUBLE_EQ(MATRIX(0, 0), SCALE.x());
  ASSERT_DOUBLE_EQ(MATRIX(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 1), SCALE.y());
}

TEST(Transform2, make_rotation)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = make_rotation(ANGLE);

  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), -std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 0), std::sin(ANGLE.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(ANGLE.rad()));
}

TEST(Transform2, make_translation)
{
  using namespace rtw::math::transform2;
  constexpr auto TRANSLATION = rtw::math::Vector2d{1.0, 2.0};
  constexpr auto MATRIX = make_translation(TRANSLATION);

  ASSERT_DOUBLE_EQ(MATRIX(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(0, 2), TRANSLATION.x());
  ASSERT_DOUBLE_EQ(MATRIX(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(MATRIX(1, 2), TRANSLATION.y());
  ASSERT_DOUBLE_EQ(MATRIX(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(MATRIX(2, 2), 1.0);
}

TEST(Transform2, make_transform)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  constexpr auto TRANSLATION = rtw::math::Vector2d{1.0, 2.0};
  const auto se2 = make_transform(ANGLE, TRANSLATION);
  ASSERT_EQ(rotation(se2), make_rotation(ANGLE));
  ASSERT_EQ(rtw::math::transform2::translation(se2), TRANSLATION);
}

TEST(Transform2, inverse_rotation)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  const auto matrix = make_rotation(ANGLE);
  const auto inverse = inverse_rotation(matrix);

  {
    ASSERT_DOUBLE_EQ(inverse(0, 0), std::cos(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 1), std::sin(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 0), -std::sin(ANGLE.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 1), std::cos(ANGLE.rad()));
  }

  {
    const auto inverse_matrix = rtw::math::inverse(matrix);
    ASSERT_EQ(inverse, inverse_matrix);
  }
}

TEST(Transform2, inverse_transform)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto ANGLE = 90.0_deg;
  constexpr auto TRANSLATION = rtw::math::Vector2d{1.0, 2.0};
  const auto se2 = make_transform(ANGLE, TRANSLATION);
  const auto inverse = inverse_transform(se2);

  ASSERT_EQ(rotation(inverse), make_rotation(-ANGLE));
  ASSERT_EQ(rtw::math::transform2::translation(inverse), -rotation(inverse) * TRANSLATION);
}
