#pragma once

#include "sw_renderer/color.h"
#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/precision.h"

#include <cstddef>

namespace rtw::sw_renderer
{

class FrameBuffer
{
public:
  FrameBuffer(const std::size_t width, const std::size_t height)
      : color_buffer_{width, height}, depth_buffer_{width, height}
  {
  }

  std::size_t width() const { return color_buffer_.width(); }
  std::size_t height() const { return color_buffer_.height(); }
  float aspect_ratio() const { return color_buffer_.aspect_ratio(); }

  ColorBuffer& color_buffer() { return color_buffer_; }
  const ColorBuffer& color_buffer() const { return color_buffer_; }

  DepthBuffer& depth_buffer() { return depth_buffer_; }
  const DepthBuffer& depth_buffer() const { return depth_buffer_; }

  void clear(const Color color, const single_precision depth)
  {
    color_buffer_.clear(color);
    depth_buffer_.clear(depth);
  }

private:
  ColorBuffer color_buffer_;
  DepthBuffer depth_buffer_;
};

} // namespace rtw::sw_renderer
