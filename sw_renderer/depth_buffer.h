#pragma once

#include <limits>
#include <vector>

namespace rtw::sw_renderer {

class DepthBuffer
{
public:
  DepthBuffer(const std::size_t width, const std::size_t height)
      : buffer_(width * height, std::numeric_limits<float>::infinity()), width_(width), height_(height)
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  std::size_t size() const { return buffer_.size(); }

  void clear() { std::fill(buffer_.begin(), buffer_.end(), std::numeric_limits<float>::infinity()); }

  void set_depth(const std::size_t x, const std::size_t y, const float depth) { buffer_[y * width_ + x] = depth; }
  float depth(const std::size_t x, const std::size_t y) const { return buffer_[y * width_ + x]; }

private:
  std::vector<float> buffer_;
  std::size_t width_{};
  std::size_t height_{};
};

} // namespace rtw::sw_renderer
