#include "math/convex_polygon.h"
#include "math/convex_polygon_operations.h"

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
  EXPECT_FALSE(polygon.is_valid());
  polygon.push_back(rtw::math::Point2I{1, 2});
  EXPECT_FALSE(polygon.is_valid());
  polygon.push_back(rtw::math::Point2I{3, 4});
  EXPECT_FALSE(polygon.is_valid());
  polygon.push_back(rtw::math::Point2I{5, 6});
  EXPECT_TRUE(polygon.is_valid());
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

TEST(ConvexPolygonTest, triangle_winding_order)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  ASSERT_EQ(rtw::math::winding_order(v0, v1, v2), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(v0, v2, v1), rtw::math::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(v1, v0, v2), rtw::math::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(v1, v2, v0), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(v2, v0, v1), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(v2, v1, v0), rtw::math::WindingOrder::CLOCKWISE);

  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v0, v1, v2}), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v0, v2, v1}), rtw::math::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v1, v0, v2}), rtw::math::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v1, v2, v0}), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v2, v0, v1}), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::math::winding_order(rtw::math::Triangle2F{v2, v1, v0}), rtw::math::WindingOrder::CLOCKWISE);
}

TEST(ConvexPolygonTest, is_convex)
{
  {
    const rtw::math::Triangle2F triangle{
        rtw::math::Point2F{0.0F, 0.0F},
        rtw::math::Point2F{1.0F, 0.0F},
        rtw::math::Point2F{0.5F, 1.0F},
    };
    ASSERT_TRUE(rtw::math::is_convex(triangle));
  }

  {
    rtw::math::ConvexPolygon2F<4U> cw_polygon;
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ASSERT_TRUE(rtw::math::is_convex(cw_polygon));
  }

  {
    rtw::math::ConvexPolygon2F<4U> convex_polygon;
    convex_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    convex_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    convex_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    convex_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    ASSERT_TRUE(rtw::math::is_convex(convex_polygon));
  }

  {
    rtw::math::ConvexPolygon2F<5U> pentagon;
    pentagon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    pentagon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    pentagon.push_back(rtw::math::Point2F{2.5F, 1.5F});
    pentagon.push_back(rtw::math::Point2F{1.0F, 2.5F});
    pentagon.push_back(rtw::math::Point2F{-0.5F, 1.5F});
    ASSERT_TRUE(rtw::math::is_convex(pentagon));
  }

  {
    rtw::math::ConvexPolygon2I<4U> int_polygon;
    int_polygon.push_back(rtw::math::Point2I{0, 0});
    int_polygon.push_back(rtw::math::Point2I{10, 0});
    int_polygon.push_back(rtw::math::Point2I{10, 10});
    int_polygon.push_back(rtw::math::Point2I{0, 10});
    ASSERT_TRUE(rtw::math::is_convex(int_polygon));
  }

  {
    // Near-collinear points that should be detected with larger epsilon
    rtw::math::ConvexPolygon2F<4U> near_collinear;
    near_collinear.push_back(rtw::math::Point2F{0.0F, 0.0F});
    // Slightly BELOW the line (maintains CCW for bottom edge)
    near_collinear.push_back(rtw::math::Point2F{1.0F, -0.0001F});
    near_collinear.push_back(rtw::math::Point2F{2.0F, 0.0F});
    near_collinear.push_back(rtw::math::Point2F{1.0F, 2.0F});

    // With default epsilon, should be CONVEX (0.0001 > epsilon)
    ASSERT_TRUE(rtw::math::is_convex(near_collinear));
    // With larger epsilon, should detect as COLLINEAR_POINTS
    const auto result = rtw::math::check_polygon(near_collinear, 0.001F);
    ASSERT_TRUE(result.has_collinear_points());
  }

  {
    rtw::math::ConvexPolygon2F<5U> concave_polygon;
    concave_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    concave_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    concave_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    concave_polygon.push_back(rtw::math::Point2F{2.0F, 2.0F});
    concave_polygon.push_back(rtw::math::Point2F{0.0F, 2.0F});
    const auto result = rtw::math::check_polygon(concave_polygon);
    ASSERT_TRUE(result.is_concave());
  }

  {
    const rtw::math::ConvexPolygon2F<4U> empty_polygon;
    auto result = rtw::math::check_polygon(empty_polygon);
    ASSERT_TRUE(result.is_invalid());

    rtw::math::ConvexPolygon2F<4U> one_vertex;
    one_vertex.push_back(rtw::math::Point2F{0.0F, 0.0F});
    result = rtw::math::check_polygon(one_vertex);
    ASSERT_TRUE(result.is_invalid());

    rtw::math::ConvexPolygon2F<4U> two_vertices;
    two_vertices.push_back(rtw::math::Point2F{0.0F, 0.0F});
    two_vertices.push_back(rtw::math::Point2F{1.0F, 0.0F});
    result = rtw::math::check_polygon(two_vertices);
    ASSERT_TRUE(result.is_invalid());
  }

  {
    // Pentagon with one collinear edge (3 consecutive points on a line)
    rtw::math::ConvexPolygon2F<5U> polygon_with_collinear;
    polygon_with_collinear.push_back(rtw::math::Point2F{0.0F, 0.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{1.0F, 0.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{2.0F, 0.0F}); // Collinear with previous two
    polygon_with_collinear.push_back(rtw::math::Point2F{2.0F, 1.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{0.0F, 1.0F});
    const auto result = rtw::math::check_polygon(polygon_with_collinear);
    ASSERT_TRUE(result.has_collinear_points());
  }

  {
    // All points collinear
    rtw::math::ConvexPolygon2F<4U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{3.0F, 0.0F});
    const auto result = rtw::math::check_polygon(degenerate_polygon);
    ASSERT_TRUE(result.has_collinear_points());
  }

  {
    // Duplicate points leading to collinearity
    rtw::math::ConvexPolygon2F<4U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    const auto result = rtw::math::check_polygon(degenerate_polygon);
    ASSERT_TRUE(result.has_collinear_points());
  }

  {
    // Duplicate points leading to collinearity
    rtw::math::ConvexPolygon2F<5U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.5F, 1.5F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 1.5F});
    degenerate_polygon.push_back(rtw::math::Point2F{-0.5F, 1.5F});
    const auto result = rtw::math::check_polygon(degenerate_polygon);
    ASSERT_TRUE(result.has_collinear_points());
  }
}

TEST(ConvexPolygonTest, winding_order)
{
  {
    const rtw::math::Triangle2F triangle{
        rtw::math::Point2F{0.0F, 0.0F},
        rtw::math::Point2F{1.0F, 0.0F},
        rtw::math::Point2F{0.5F, 1.0F},
    };
    ASSERT_EQ(rtw::math::winding_order(triangle), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  }

  {
    rtw::math::ConvexPolygon2F<4U> cw_polygon;
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ASSERT_EQ(rtw::math::winding_order(cw_polygon), rtw::math::WindingOrder::CLOCKWISE);
  }

  {
    rtw::math::ConvexPolygon2F<4U> ccw_polygon;
    ccw_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    ccw_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ccw_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    ccw_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    ASSERT_EQ(rtw::math::winding_order(ccw_polygon), rtw::math::WindingOrder::COUNTER_CLOCKWISE);
  }
}
