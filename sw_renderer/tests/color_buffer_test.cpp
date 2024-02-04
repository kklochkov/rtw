#include "sw_renderer/color_buffer.h"

#include <gtest/gtest.h>

TEST(Color, default_constructor)
{
  constexpr rtw::sw_renderer::Color color{};
  EXPECT_EQ(color.r(), 0x00);
  EXPECT_EQ(color.g(), 0x00);
  EXPECT_EQ(color.b(), 0x00);
  EXPECT_EQ(color.a(), 0xFF);
  EXPECT_EQ(color.rgba, 0x000000FF);
}

TEST(Color, constructor)
{
  {
    constexpr rtw::sw_renderer::Color color{0x12345678};
    EXPECT_EQ(color.r(), 0x12);
    EXPECT_EQ(color.g(), 0x34);
    EXPECT_EQ(color.b(), 0x56);
    EXPECT_EQ(color.a(), 0x78);
    EXPECT_EQ(color.rgba, 0x12345678);
  }
  {
    constexpr rtw::sw_renderer::Color color{0x12, 0x34, 0x56, 0x78};
    EXPECT_EQ(color.r(), 0x12);
    EXPECT_EQ(color.g(), 0x34);
    EXPECT_EQ(color.b(), 0x56);
    EXPECT_EQ(color.a(), 0x78);
  }
  {
    constexpr rtw::sw_renderer::Color color{0.2F, 0.4F, 0.8F, 1.0F};
    EXPECT_EQ(color.r(), 0x33);
    EXPECT_EQ(color.g(), 0x66);
    EXPECT_EQ(color.b(), 0xCC);
    EXPECT_EQ(color.a(), 0xFF);
    EXPECT_EQ(color.rgba, 0x3366CCFF);
    EXPECT_EQ(color.rf(), 0.2F);
    EXPECT_EQ(color.gf(), 0.4F);
    EXPECT_EQ(color.bf(), 0.8F);
    EXPECT_EQ(color.af(), 1.0F);
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
  EXPECT_EQ(color.rgba, 0x12345678);

  color.set_rf(0.2F);
  color.set_gf(0.4F);
  color.set_bf(0.8F);
  color.set_af(1.0F);
  EXPECT_EQ(color.r(), 0x33);
  EXPECT_EQ(color.g(), 0x66);
  EXPECT_EQ(color.b(), 0xCC);
  EXPECT_EQ(color.a(), 0xFF);
  EXPECT_EQ(color.rgba, 0x3366CCFF);
}

TEST(Color, invert)
{
  constexpr rtw::sw_renderer::Color color{0x12345678};
  EXPECT_EQ(color.invert().rgba, 0xEDCBA978);
}

TEST(Color, multiply)
{
  constexpr rtw::sw_renderer::Color color{0x12345678};
  EXPECT_EQ((color * 0.5F).rgba, 0x091A2B78);
}

TEST(Color, interpolate)
{
  constexpr rtw::sw_renderer::Color color1{0x12345678};
  constexpr rtw::sw_renderer::Color color2{0x9ABCDEF0};
  EXPECT_EQ(rtw::sw_renderer::lerp(color1, color2, 0.5F).rgba, 0x567899B4);
}

TEST(Color, operator_stream)
{
  constexpr rtw::sw_renderer::Color color{0x123456FF};
  std::ostringstream oss;
  oss << color;
  constexpr auto expected =
      "Color(rgba: 0x123456FF, r: 18, g: 52, b: 86, a: 255, rf: 0.0706, gf: 0.2039, bf: 0.3373, af: 1.0000)\n";
  EXPECT_EQ(oss.str(), expected);
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
  color_buffer.set_pixel(0, 0, rtw::sw_renderer::Color{0x12345678});
  EXPECT_EQ(color_buffer.pixel(0, 0).rgba, 0x12345678);
}
