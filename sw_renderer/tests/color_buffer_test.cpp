#include "sw_renderer/color_buffer.h"
#include "sw_renderer/format.h"

#include <gtest/gtest.h>

TEST(Color, default_constructor)
{
  constexpr rtw::sw_renderer::Color COLOR{};
  EXPECT_EQ(COLOR.r(), 0x00);
  EXPECT_EQ(COLOR.g(), 0x00);
  EXPECT_EQ(COLOR.b(), 0x00);
  EXPECT_EQ(COLOR.a(), 0xFF);
  EXPECT_EQ(COLOR.rgba, 0x00'00'00'FF);
}

TEST(Color, constructor)
{
  {
    constexpr rtw::sw_renderer::Color COLOR{0x12'34'56'78};
    EXPECT_EQ(COLOR.r(), 0x12);
    EXPECT_EQ(COLOR.g(), 0x34);
    EXPECT_EQ(COLOR.b(), 0x56);
    EXPECT_EQ(COLOR.a(), 0x78);
    EXPECT_EQ(COLOR.rgba, 0x12'34'56'78);
  }
  {
    constexpr rtw::sw_renderer::Color COLOR{0x12U, 0x34U, 0x56U, 0x78U};
    EXPECT_EQ(COLOR.r(), 0x12);
    EXPECT_EQ(COLOR.g(), 0x34);
    EXPECT_EQ(COLOR.b(), 0x56);
    EXPECT_EQ(COLOR.a(), 0x78);
  }
  {
    constexpr rtw::sw_renderer::Color COLOR{0.2F, 0.4F, 0.8F, 1.0F};
    EXPECT_EQ(COLOR.r(), 0x33);
    EXPECT_EQ(COLOR.g(), 0x66);
    EXPECT_EQ(COLOR.b(), 0xCC);
    EXPECT_EQ(COLOR.a(), 0xFF);
    EXPECT_EQ(COLOR.rgba, 0x33'66'CC'FF);
    EXPECT_EQ(COLOR.rf(), 0.2F);
    EXPECT_EQ(COLOR.gf(), 0.4F);
    EXPECT_EQ(COLOR.bf(), 0.8F);
    EXPECT_EQ(COLOR.af(), 1.0F);
  }
}

TEST(Color, set_and_get)
{
  rtw::sw_renderer::Color color{};
  color.set_r(0x12);
  color.set_g(0x34);
  color.set_b(0x56);
  color.set_a(0x78);
  EXPECT_EQ(color.r(), 0x12);
  EXPECT_EQ(color.g(), 0x34);
  EXPECT_EQ(color.b(), 0x56);
  EXPECT_EQ(color.a(), 0x78);
  EXPECT_EQ(color.rgba, 0x12'34'56'78);

  color.set_rf(0.2F);
  color.set_gf(0.4F);
  color.set_bf(0.8F);
  color.set_af(1.0F);
  EXPECT_EQ(color.r(), 0x33);
  EXPECT_EQ(color.g(), 0x66);
  EXPECT_EQ(color.b(), 0xCC);
  EXPECT_EQ(color.a(), 0xFF);
  EXPECT_EQ(color.rgba, 0x33'66'CC'FF);
}

TEST(Color, invert)
{
  constexpr rtw::sw_renderer::Color COLOR{0x12'34'56'78};
  EXPECT_EQ(COLOR.invert().rgba, 0xED'CB'A9'78);
}

TEST(Color, multiply)
{
  constexpr rtw::sw_renderer::Color COLOR{0x12'34'56'78};
  EXPECT_EQ((COLOR * 0.5F).rgba, 0x09'1A'2B'78);
}

TEST(Color, interpolate)
{
  constexpr rtw::sw_renderer::Color COLOR1{0x12'34'56'78};
  constexpr rtw::sw_renderer::Color COLOR2{0x9A'BC'DE'F0};
  EXPECT_EQ(rtw::sw_renderer::lerp(COLOR1, COLOR2, 0.5F).rgba, 0x56'78'99'B4);
}

TEST(Color, operator_stream)
{
  constexpr rtw::sw_renderer::Color COLOR{0x12'34'56'FF};
  std::ostringstream oss;
  oss << COLOR;
  constexpr auto EXPECTED =
      "Color(rgba: 0x123456FF, r: 18, g: 52, b: 86, a: 255, rf: 0.0706, gf: 0.2039, bf: 0.3373, af: 1.0000)\n";
  EXPECT_EQ(oss.str(), EXPECTED);
}

TEST(ColorBuffer, default_constructor)
{
  const rtw::sw_renderer::ColorBuffer color_buffer{640, 480};
  EXPECT_EQ(color_buffer.width(), 640);
  EXPECT_EQ(color_buffer.height(), 480);
  EXPECT_EQ(color_buffer.aspect_ratio(), 640.0F / 480.0F);
  EXPECT_EQ(color_buffer.size(), 640 * 480);
  EXPECT_EQ(color_buffer.bytes_per_pixel(), sizeof(std::uint32_t));
  EXPECT_EQ(color_buffer.pitch(), 640 * sizeof(std::uint32_t));
  EXPECT_EQ(*color_buffer.data(), color_buffer.pixel(0, 0).rgba);
  EXPECT_EQ(color_buffer.pixel(0, 0).rgba, rtw::sw_renderer::Color{}.rgba);
}

TEST(ColorBuffer, set_and_get_pixel)
{
  rtw::sw_renderer::ColorBuffer color_buffer{640, 480};
  color_buffer.set_pixel(0, 0, rtw::sw_renderer::Color{0x12'34'56'78});
  EXPECT_EQ(color_buffer.pixel(0, 0).rgba, 0x12'34'56'78);
}
