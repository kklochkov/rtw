#pragma once

#include "sw_renderer/color.h"

#include <vector>

namespace rtw::sw_renderer
{

class ColorBuffer
{
public:
  ColorBuffer(const std::size_t width, const std::size_t height)
      : buffer_(width * height, Color().rgba), width_(width), height_(height), pitch_(width * bytes_per_pixel_),
        aspect_ratio_(static_cast<float>(width) / static_cast<float>(height))
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  float aspect_ratio() const { return aspect_ratio_; }
  std::size_t size() const { return buffer_.size(); }
  std::size_t bytes_per_pixel() const { return bytes_per_pixel_; }
  std::size_t pitch() const { return pitch_; }

  void set_pixel(const std::size_t x, const std::size_t y, const Color color) { buffer_[y * width_ + x] = color.rgba; }
  Color pixel(const std::size_t x, const std::size_t y) const { return Color{buffer_[y * width_ + x]}; }

  void clear(const Color color) { std::fill(buffer_.begin(), buffer_.end(), color.rgba); }

  const std::uint32_t* data() const { return buffer_.data(); }

private:
  std::vector<std::uint32_t> buffer_;
  std::size_t width_{};
  std::size_t height_{};
  std::size_t bytes_per_pixel_{sizeof(std::uint32_t)};
  std::size_t pitch_{};
  float aspect_ratio_{1.0F};
};

} // namespace rtw::sw_renderer
