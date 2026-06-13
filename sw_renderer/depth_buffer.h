#pragma once

#include "sw_renderer/precision.h"

#include <cassert>
#include <limits>
#include <vector>

namespace rtw::sw_renderer
{

/// A 2D buffer for storing depth (Z) values.
/// Used for depth testing to determine visibility.
///
/// The buffer stores depth values in single_precision (float, or fixed-point when
/// RTW_USE_FIXED_POINT is enabled), initialised to std::numeric_limits<single_precision>::max()
/// (farthest possible). Smaller values are closer to the camera. Storing the same scalar type the
/// rasteriser interpolates avoids a per-pixel conversion in the depth test.
///
/// @note Uses inverted depth (1/w) for better precision.
class DepthBuffer
{
public:
  DepthBuffer(const std::size_t width, const std::size_t height)
      : buffer_(width * height, std::numeric_limits<single_precision>::max()), width_(width), height_(height)
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  std::size_t size() const { return buffer_.size(); }

  void clear(const single_precision value = std::numeric_limits<single_precision>::max())
  {
    std::fill(buffer_.begin(), buffer_.end(), value);
  }

  void set_depth(const std::size_t x, const std::size_t y, const single_precision depth)
  {
    assert(x < width_ && "x coordinate out of bounds");
    assert(y < height_ && "y coordinate out of bounds");
    buffer_[(y * width_) + x] = depth;
  }

  single_precision depth(const std::size_t x, const std::size_t y) const
  {
    assert(x < width_ && "x coordinate out of bounds");
    assert(y < height_ && "y coordinate out of bounds");
    return buffer_[(y * width_) + x];
  }

private:
  std::vector<single_precision> buffer_;
  std::size_t width_{};
  std::size_t height_{};
};

} // namespace rtw::sw_renderer
