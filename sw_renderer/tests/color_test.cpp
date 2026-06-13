#include "sw_renderer/color.h"
#include "sw_renderer/types.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
namespace sw = rtw::sw_renderer;
} // namespace

TEST(Color, cast_to_vector4)
{
  // R=255, G=128, B=0, A=255
  constexpr sw::Color COLOR{0xFF'80'00'FFU};
  constexpr auto VECTOR = static_cast<sw::Vector4F>(COLOR);
  EXPECT_FLOAT_EQ(VECTOR.x(), 1.0F);
  EXPECT_FLOAT_EQ(VECTOR.y(), 128.0F / 255.0F);
  EXPECT_FLOAT_EQ(VECTOR.z(), 0.0F);
  EXPECT_FLOAT_EQ(VECTOR.w(), 1.0F);
}

TEST(Color, construct_from_vector4)
{
  constexpr sw::Color COLOR{sw::Vector4F{1.0F, 0.0F, 0.0F, 1.0F}};
  EXPECT_EQ(COLOR, sw::Color{0xFF'00'00'FFU});
}

TEST(Color, construct_from_vector4_clamps_and_truncates)
{
  // 2.0 -> 1.0 (clamped) -> 255; -1.0 -> 0.0 (clamped) -> 0;
  // 0.5 -> 127.5 -> 127 (truncated); 10.0 -> 1.0 (clamped) -> 255.
  constexpr sw::Color COLOR{sw::Vector4F{2.0F, -1.0F, 0.5F, 10.0F}};
  EXPECT_EQ(COLOR, sw::Color{0xFF'00'7F'FFU});
}

TEST(Color, round_trip_is_lossy)
{
  // Channels that are not exact multiples of 1/255 are truncated when converted back to 8 bits:
  // 0.5 -> 127 (not 128), so the round trip is not the identity.
  constexpr sw::Color COLOR{sw::Vector4F{0.5F, 0.5F, 0.5F, 1.0F}};
  EXPECT_EQ(COLOR, sw::Color{0x7F'7F'7F'FFU});
}
