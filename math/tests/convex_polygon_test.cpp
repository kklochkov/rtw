#include "math/convex_polygon.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ConvexPolygonTest, default_ctor)
{
  constexpr rtw::math::ConvexPolygon2I<3U> POLYGON;
  EXPECT_EQ(POLYGON.size(), 0U);
  EXPECT_EQ(POLYGON.capacity(), 3U);
}

TEST(ConvexPolygonTest, initializer_list_constructor)
{
  constexpr rtw::math::ConvexPolygon2I<3U> POLYGON = {rtw::math::Point2I{1, 2}, rtw::math::Point2I{3, 4},
                                                      rtw::math::Point2I{5, 6}};
  EXPECT_EQ(POLYGON.size(), 3U);
  EXPECT_EQ(POLYGON.capacity(), 3U);
  EXPECT_EQ(POLYGON[0], rtw::math::Point2I(1, 2));
  EXPECT_EQ(POLYGON[1], rtw::math::Point2I(3, 4));
  EXPECT_EQ(POLYGON[2], rtw::math::Point2I(5, 6));
  EXPECT_EQ(POLYGON.size(), 3U);
  EXPECT_EQ(POLYGON.capacity(), 3U);
}

TEST(ConvexPolygonTest, push_back)
{
  rtw::math::ConvexPolygon2I<3U> polygon;
  polygon.push_back(rtw::math::Point2I{1, 2});
  polygon.push_back(rtw::math::Point2I{3, 4});
  polygon.push_back(rtw::math::Point2I{5, 6});
  EXPECT_EQ(polygon.size(), 3U);
  EXPECT_EQ(polygon.capacity(), 3U);
  EXPECT_EQ(polygon[0], rtw::math::Point2I(1, 2));
  EXPECT_EQ(polygon[1], rtw::math::Point2I(3, 4));
  EXPECT_EQ(polygon[2], rtw::math::Point2I(5, 6));
}

TEST(ConvexPolygonTest, clear)
{
  rtw::math::ConvexPolygon2I<3U> polygon = {rtw::math::Point2I{1, 2}, rtw::math::Point2I{3, 4},
                                            rtw::math::Point2I{5, 6}};
  polygon.clear();
  EXPECT_EQ(polygon.size(), 0U);
  EXPECT_EQ(polygon.capacity(), 3U);
}

TEST(ConvexPolygonTest, begin_end)
{
  constexpr rtw::math::ConvexPolygon2I<3U> POLYGON = {rtw::math::Point2I{1, 2}, rtw::math::Point2I{3, 4},
                                                      rtw::math::Point2I{5, 6}};
  EXPECT_EQ(*POLYGON.begin(), rtw::math::Point2I(1, 2));
  EXPECT_EQ(*(POLYGON.begin() + 1), rtw::math::Point2I(3, 4));
  EXPECT_EQ(*(POLYGON.begin() + 2), rtw::math::Point2I(5, 6));
  EXPECT_EQ(POLYGON.begin() + 3, POLYGON.end());
}

TEST(ConvexPolygonTest, valid)
{
  rtw::math::ConvexPolygon2I<3U> polygon;
  EXPECT_FALSE(polygon.valid());
  polygon.push_back(rtw::math::Point2I{1, 2});
  EXPECT_FALSE(polygon.valid());
  polygon.push_back(rtw::math::Point2I{3, 4});
  EXPECT_FALSE(polygon.valid());
  polygon.push_back(rtw::math::Point2I{5, 6});
  EXPECT_TRUE(polygon.valid());
}

TEST(ConvexPolygonTest, index_operator)
{
  constexpr rtw::math::ConvexPolygon2I<3U> POLYGON = {rtw::math::Point2I{1, 2}, rtw::math::Point2I{3, 4},
                                                      rtw::math::Point2I{5, 6}};
  EXPECT_EQ(POLYGON[0], rtw::math::Point2I(1, 2));
  EXPECT_EQ(POLYGON[1], rtw::math::Point2I(3, 4));
  EXPECT_EQ(POLYGON[2], rtw::math::Point2I(5, 6));
}

TEST(ConvexPolygonTest, index_operator_out_of_bounds)
{
  constexpr rtw::math::ConvexPolygon2I<3U> POLYGON = {rtw::math::Point2I{1, 2}, rtw::math::Point2I{3, 4},
                                                      rtw::math::Point2I{5, 6}};
  EXPECT_DEATH(POLYGON[3], "");
}
