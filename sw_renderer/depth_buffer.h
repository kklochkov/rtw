#pragma once

#include <cassert>
#include <limits>
#include <vector>

namespace rtw::sw_renderer
{

/// A 2D buffer for storing depth (Z) values.
/// Used for depth testing to determine visibility.
///
/// The buffer stores floating-point depth values, initialized to
/// std::numeric_limits<float>::max() (farthest possible).
/// Smaller values are closer to the camera.
///
/// @note Uses inverted depth (1/w) for better floating-point precision.
class DepthBuffer
{
public:
  DepthBuffer(const std::size_t width, const std::size_t height)
      : buffer_(width * height, std::numeric_limits<float>::max()), width_(width), height_(height)
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  std::size_t size() const { return buffer_.size(); }

  void clear() { std::fill(buffer_.begin(), buffer_.end(), std::numeric_limits<float>::max()); }

  void set_depth(const std::size_t x, const std::size_t y, const float depth)
  {
    assert(x < width_ && "x coordinate out of bounds");
    assert(y < height_ && "y coordinate out of bounds");
    buffer_[(y * width_) + x] = depth;
  }

  float depth(const std::size_t x, const std::size_t y) const
  {
    assert(x < width_ && "x coordinate out of bounds");
    assert(y < height_ && "y coordinate out of bounds");
    return buffer_[(y * width_) + x];
  }

private:
  std::vector<float> buffer_;
  std::size_t width_{};
  std::size_t height_{};
};

} // namespace rtw::sw_renderer
