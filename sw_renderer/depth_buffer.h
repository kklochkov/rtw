#pragma once

#include "fixed_point/fixed_point.h"

#include <limits>
#include <vector>

namespace rtw::sw_renderer
{

template <typename T>
class GenericDepthBuffer
{
public:
  GenericDepthBuffer(const std::size_t width, const std::size_t height)
      : buffer_(width * height, std::numeric_limits<T>::max()), width_(width), height_(height)
  {
  }

  std::size_t width() const { return width_; }
  std::size_t height() const { return height_; }
  std::size_t size() const { return buffer_.size(); }

  void clear() { std::fill(buffer_.begin(), buffer_.end(), std::numeric_limits<T>::max()); }

  void set_depth(const std::size_t x, const std::size_t y, const T depth) { buffer_[y * width_ + x] = depth; }
  T depth(const std::size_t x, const std::size_t y) const { return buffer_[y * width_ + x]; }

private:
  std::vector<T> buffer_;
  std::size_t width_{};
  std::size_t height_{};
};

using DepthBuffer = GenericDepthBuffer<float>;
using DepthBufferQ16 = GenericDepthBuffer<fixed_point::FixedPoint16>;
using DepthBufferQ32 = GenericDepthBuffer<fixed_point::FixedPoint32>;

} // namespace rtw::sw_renderer
