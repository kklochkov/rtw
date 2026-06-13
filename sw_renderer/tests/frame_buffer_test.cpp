#include "sw_renderer/color.h"
#include "sw_renderer/frame_buffer.h"

#include <gtest/gtest.h>

#include <limits>

namespace
{
namespace sw = rtw::sw_renderer;
} // namespace

TEST(FrameBuffer, construction)
{
  const sw::FrameBuffer frame_buffer{640, 480};
  EXPECT_EQ(frame_buffer.width(), 640);
  EXPECT_EQ(frame_buffer.height(), 480);
  EXPECT_EQ(frame_buffer.aspect_ratio(), 640.0F / 480.0F);

  EXPECT_EQ(frame_buffer.color_buffer().width(), 640);
  EXPECT_EQ(frame_buffer.color_buffer().height(), 480);
  EXPECT_EQ(frame_buffer.depth_buffer().width(), 640);
  EXPECT_EQ(frame_buffer.depth_buffer().height(), 480);
}

TEST(FrameBuffer, initial_depth_is_farthest)
{
  const sw::FrameBuffer frame_buffer{8, 8};
  EXPECT_EQ(frame_buffer.depth_buffer().depth(0, 0), std::numeric_limits<sw::single_precision>::max());
  EXPECT_EQ(frame_buffer.depth_buffer().depth(7, 7), std::numeric_limits<sw::single_precision>::max());
}

TEST(FrameBuffer, clear_sets_color_and_depth)
{
  sw::FrameBuffer frame_buffer{16, 16};
  constexpr sw::Color CLEAR_COLOR{0x12'34'56'78U};
  constexpr sw::single_precision CLEAR_DEPTH{1};

  // Dirty a few pixels first.
  frame_buffer.color_buffer().set_pixel(0, 0, sw::Color{0xFF'FF'FF'FFU});
  frame_buffer.depth_buffer().set_depth(0, 0, sw::single_precision{0});

  frame_buffer.clear(CLEAR_COLOR, CLEAR_DEPTH);

  // Corners + center.
  EXPECT_EQ(frame_buffer.color_buffer().pixel(0, 0), CLEAR_COLOR);
  EXPECT_EQ(frame_buffer.color_buffer().pixel(15, 0), CLEAR_COLOR);
  EXPECT_EQ(frame_buffer.color_buffer().pixel(0, 15), CLEAR_COLOR);
  EXPECT_EQ(frame_buffer.color_buffer().pixel(15, 15), CLEAR_COLOR);
  EXPECT_EQ(frame_buffer.color_buffer().pixel(8, 8), CLEAR_COLOR);

  EXPECT_EQ(frame_buffer.depth_buffer().depth(0, 0), CLEAR_DEPTH);
  EXPECT_EQ(frame_buffer.depth_buffer().depth(15, 15), CLEAR_DEPTH);
  EXPECT_EQ(frame_buffer.depth_buffer().depth(8, 8), CLEAR_DEPTH);
}

TEST(FrameBuffer, mutable_accessors_write_through)
{
  sw::FrameBuffer frame_buffer{4, 4};
  frame_buffer.color_buffer().set_pixel(1, 2, sw::Color{0xAA'BB'CC'DDU});
  frame_buffer.depth_buffer().set_depth(1, 2, sw::single_precision{0.25F});

  const sw::FrameBuffer& const_ref = frame_buffer;
  EXPECT_EQ(const_ref.color_buffer().pixel(1, 2), sw::Color{0xAA'BB'CC'DDU});
  EXPECT_EQ(const_ref.depth_buffer().depth(1, 2), sw::single_precision{0.25F});
}
