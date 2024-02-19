#pragma once

#include "sw_renderer/color.h"

#include <vector>

namespace rtw::sw_renderer
{

class Texture
{
public:
  Texture() = default;
  Texture(std::uint32_t* data, const std::size_t width, const std::size_t height)
      : buffer_(data, data + width * height), // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        width_(width), height_(height), pitch_(width * bytes_per_pixel_)
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  std::size_t size() const { return buffer_.size(); }
  std::size_t bytes_per_pixel() const { return bytes_per_pixel_; }
  std::size_t pitch() const { return pitch_; }

  Color texel(const std::size_t x, const std::size_t y) const { return Color{buffer_[y * width_ + x]}; }

private:
  std::vector<std::uint32_t> buffer_;
  std::size_t width_{};
  std::size_t height_{};
  std::size_t bytes_per_pixel_{sizeof(std::uint32_t)};
  std::size_t pitch_{};
};

} // namespace rtw::sw_renderer
