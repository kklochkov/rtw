#pragma once

#include <cstddef>

namespace rtw::sw_renderer
{

/// Statistics collected during rendering.
struct RenderStats
{
  std::size_t triangles_submitted{0}; ///< Triangles before clipping
  std::size_t triangles_clipped{0};   ///< Triangles fully outside frustum
  std::size_t triangles_culled{0};    ///< Triangles removed by face culling
  std::size_t triangles_rendered{0};  ///< Triangles actually drawn

  void reset() noexcept
  {
    triangles_submitted = 0;
    triangles_clipped = 0;
    triangles_culled = 0;
    triangles_rendered = 0;
  }
};

} // namespace rtw::sw_renderer
