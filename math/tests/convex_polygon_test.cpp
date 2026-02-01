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

TEST(ConvexPolygonTest, winding_order)
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
    ASSERT_EQ(rtw::math::is_convex(triangle), rtw::math::ConvexityCheckResult::CONVEX);
  }

  {
    rtw::math::ConvexPolygon2F<4U> cw_polygon;
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    cw_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    cw_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ASSERT_EQ(rtw::math::is_convex(cw_polygon), rtw::math::ConvexityCheckResult::CONVEX);
  }

  {
    rtw::math::ConvexPolygon2F<4U> convex_polygon;
    convex_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    convex_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    convex_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    convex_polygon.push_back(rtw::math::Point2F{0.0F, 1.0F});
    ASSERT_EQ(rtw::math::is_convex(convex_polygon), rtw::math::ConvexityCheckResult::CONVEX);
  }

  {
    rtw::math::ConvexPolygon2F<5U> pentagon;
    pentagon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    pentagon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    pentagon.push_back(rtw::math::Point2F{2.5F, 1.5F});
    pentagon.push_back(rtw::math::Point2F{1.0F, 2.5F});
    pentagon.push_back(rtw::math::Point2F{-0.5F, 1.5F});
    ASSERT_EQ(rtw::math::is_convex(pentagon), rtw::math::ConvexityCheckResult::CONVEX);
  }

  {
    rtw::math::ConvexPolygon2I<4U> int_polygon;
    int_polygon.push_back(rtw::math::Point2I{0, 0});
    int_polygon.push_back(rtw::math::Point2I{10, 0});
    int_polygon.push_back(rtw::math::Point2I{10, 10});
    int_polygon.push_back(rtw::math::Point2I{0, 10});
    ASSERT_EQ(rtw::math::is_convex(int_polygon), rtw::math::ConvexityCheckResult::CONVEX);
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
    ASSERT_EQ(rtw::math::is_convex(near_collinear), rtw::math::ConvexityCheckResult::CONVEX);
    // With larger epsilon, should detect as COLLINEAR_POINTS
    ASSERT_EQ(rtw::math::is_convex(near_collinear, 0.001F), rtw::math::ConvexityCheckResult::COLLINEAR_POINTS);
  }

  {
    rtw::math::ConvexPolygon2F<5U> concave_polygon;
    concave_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    concave_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    concave_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    concave_polygon.push_back(rtw::math::Point2F{2.0F, 2.0F});
    concave_polygon.push_back(rtw::math::Point2F{0.0F, 2.0F});
    ASSERT_EQ(rtw::math::is_convex(concave_polygon), rtw::math::ConvexityCheckResult::CONCAVE);
  }

  {
    const rtw::math::ConvexPolygon2F<4U> empty_polygon;
    ASSERT_EQ(rtw::math::is_convex(empty_polygon), rtw::math::ConvexityCheckResult::INVALID_POLYGON);

    rtw::math::ConvexPolygon2F<4U> one_vertex;
    one_vertex.push_back(rtw::math::Point2F{0.0F, 0.0F});
    ASSERT_EQ(rtw::math::is_convex(one_vertex), rtw::math::ConvexityCheckResult::INVALID_POLYGON);

    rtw::math::ConvexPolygon2F<4U> two_vertices;
    two_vertices.push_back(rtw::math::Point2F{0.0F, 0.0F});
    two_vertices.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ASSERT_EQ(rtw::math::is_convex(two_vertices), rtw::math::ConvexityCheckResult::INVALID_POLYGON);
  }

  {
    // Pentagon with one collinear edge (3 consecutive points on a line)
    rtw::math::ConvexPolygon2F<5U> polygon_with_collinear;
    polygon_with_collinear.push_back(rtw::math::Point2F{0.0F, 0.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{1.0F, 0.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{2.0F, 0.0F}); // Collinear with previous two
    polygon_with_collinear.push_back(rtw::math::Point2F{2.0F, 1.0F});
    polygon_with_collinear.push_back(rtw::math::Point2F{0.0F, 1.0F});
    ASSERT_EQ(rtw::math::is_convex(polygon_with_collinear), rtw::math::ConvexityCheckResult::COLLINEAR_POINTS);
  }

  {
    // All points collinear
    rtw::math::ConvexPolygon2F<4U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{3.0F, 0.0F});
    ASSERT_EQ(rtw::math::is_convex(degenerate_polygon), rtw::math::ConvexityCheckResult::COLLINEAR_POINTS);
  }

  {
    // Duplicate points leading to collinearity
    rtw::math::ConvexPolygon2F<4U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 1.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{1.0F, 0.0F});
    ASSERT_EQ(rtw::math::is_convex(degenerate_polygon), rtw::math::ConvexityCheckResult::COLLINEAR_POINTS);
  }

  {
    // Duplicate points leading to collinearity
    rtw::math::ConvexPolygon2F<5U> degenerate_polygon;
    degenerate_polygon.push_back(rtw::math::Point2F{0.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 0.0F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.5F, 1.5F});
    degenerate_polygon.push_back(rtw::math::Point2F{2.0F, 1.5F});
    degenerate_polygon.push_back(rtw::math::Point2F{-0.5F, 1.5F});
    ASSERT_EQ(rtw::math::is_convex(degenerate_polygon), rtw::math::ConvexityCheckResult::COLLINEAR_POINTS);
  }
}
