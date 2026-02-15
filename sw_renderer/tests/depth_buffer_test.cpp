#include "sw_renderer/depth_buffer.h"

#include <gtest/gtest.h>

#include <limits>

namespace rtw::sw_renderer
{
namespace
{

TEST(DepthBuffer, construction)
{
  const DepthBuffer buffer{640, 480};
  EXPECT_EQ(buffer.width(), 640);
  EXPECT_EQ(buffer.height(), 480);
  EXPECT_EQ(buffer.size(), 640 * 480);
}

TEST(DepthBuffer, initial_values)
{
  const DepthBuffer buffer{10, 10};
  constexpr auto MAX_DEPTH = std::numeric_limits<float>::max();

  for (std::size_t y = 0; y < buffer.height(); ++y)
  {
    for (std::size_t x = 0; x < buffer.width(); ++x)
    {
      EXPECT_EQ(buffer.depth(x, y), MAX_DEPTH);
    }
  }
}

TEST(DepthBuffer, set_and_get)
{
  DepthBuffer buffer{10, 10};

  buffer.set_depth(5, 5, 0.5F);
  EXPECT_EQ(buffer.depth(5, 5), 0.5F);

  buffer.set_depth(5, 5, 0.25F);
  EXPECT_EQ(buffer.depth(5, 5), 0.25F);
}

TEST(DepthBuffer, clear)
{
  DepthBuffer buffer{10, 10};
  constexpr auto MAX_DEPTH = std::numeric_limits<float>::max();

  buffer.set_depth(3, 3, 0.1F);
  buffer.set_depth(7, 7, 0.9F);

  buffer.clear();

  EXPECT_EQ(buffer.depth(3, 3), MAX_DEPTH);
  EXPECT_EQ(buffer.depth(7, 7), MAX_DEPTH);
}

TEST(DepthBuffer, multiple_pixels)
{
  DepthBuffer buffer{10, 10};

  buffer.set_depth(0, 0, 0.1F);
  buffer.set_depth(5, 5, 0.5F);
  buffer.set_depth(9, 9, 0.9F);

  EXPECT_EQ(buffer.depth(0, 0), 0.1F);
  EXPECT_EQ(buffer.depth(5, 5), 0.5F);
  EXPECT_EQ(buffer.depth(9, 9), 0.9F);
}

TEST(DepthBuffer, corner_pixels)
{
  DepthBuffer buffer{100, 80};

  buffer.set_depth(0, 0, 0.1F);
  buffer.set_depth(99, 0, 0.2F);
  buffer.set_depth(0, 79, 0.3F);
  buffer.set_depth(99, 79, 0.4F);

  EXPECT_EQ(buffer.depth(0, 0), 0.1F);
  EXPECT_EQ(buffer.depth(99, 0), 0.2F);
  EXPECT_EQ(buffer.depth(0, 79), 0.3F);
  EXPECT_EQ(buffer.depth(99, 79), 0.4F);
}

} // namespace
} // namespace rtw::sw_renderer
