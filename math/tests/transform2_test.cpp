#include "math/transform2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(Transform2, make_scale)
{
  using namespace rtw::math::transform2;
  constexpr auto scale = rtw::math::Vector2d{1.0, 2.0};
  constexpr auto matrix = make_scale(scale);

  ASSERT_DOUBLE_EQ(matrix(0, 0), scale.x());
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), scale.y());
}

TEST(Transform2, make_rotation)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = make_rotation(angle);

  ASSERT_DOUBLE_EQ(matrix(0, 0), std::cos(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(0, 1), -std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 0), std::sin(angle.rad()));
  ASSERT_DOUBLE_EQ(matrix(1, 1), std::cos(angle.rad()));
}

TEST(Transform2, make_translation)
{
  using namespace rtw::math::transform2;
  constexpr auto translation = rtw::math::Vector2d{1.0, 2.0};
  constexpr auto matrix = make_translation(translation);

  ASSERT_DOUBLE_EQ(matrix(0, 0), 1.0);
  ASSERT_DOUBLE_EQ(matrix(0, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(0, 2), translation.x());
  ASSERT_DOUBLE_EQ(matrix(1, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(1, 1), 1.0);
  ASSERT_DOUBLE_EQ(matrix(1, 2), translation.y());
  ASSERT_DOUBLE_EQ(matrix(2, 0), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 1), 0.0);
  ASSERT_DOUBLE_EQ(matrix(2, 2), 1.0);
}

TEST(Transform2, make_transform)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  constexpr auto translation = rtw::math::Vector2d{1.0, 2.0};
  const auto se2 = make_transform(angle, translation);
  ASSERT_EQ(rotation(se2), make_rotation(angle));
  ASSERT_EQ(rtw::math::transform2::translation(se2), translation);
}

TEST(Transform2, inverse_rotation)
{
  using namespace rtw::math::transform2;
  using namespace rtw::math::angle_literals;
  constexpr auto angle = 90.0_deg;
  const auto matrix = make_rotation(angle);
  const auto inverse = inverse_rotation(matrix);

  {
    ASSERT_DOUBLE_EQ(inverse(0, 0), std::cos(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(0, 1), std::sin(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 0), -std::sin(angle.rad()));
    ASSERT_DOUBLE_EQ(inverse(1, 1), std::cos(angle.rad()));
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
  constexpr auto angle = 90.0_deg;
  constexpr auto translation = rtw::math::Vector2d{1.0, 2.0};
  const auto se2 = make_transform(angle, translation);
  const auto inverse = inverse_transform(se2);

  ASSERT_EQ(rotation(inverse), make_rotation(-angle));
  ASSERT_EQ(rtw::math::transform2::translation(inverse), -rotation(inverse) * translation);
}
