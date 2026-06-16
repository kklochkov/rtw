#pragma once

#include "sw_renderer/clipping.h"
#include "sw_renderer/color.h"
#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/programmable_pipeline/clip_space.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline_rasterisation.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/register_file.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/types.h"

#include "math/vector.h"
#include "math/vector_operations.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{

class Pipeline
{
public:
  void draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                   FrameBuffer& framebuffer, RenderStats& stats);

  void draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                     const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats);

private:
  void transform_vertices(const IShaderProgram& program, const RawVertexStream& vertices);

  static void process_triangle(const IShaderProgram& program, const ClipVertex<single_precision>& v0,
                               const ClipVertex<single_precision>& v1, const ClipVertex<single_precision>& v2,
                               std::uint32_t primitive_id, const PipelineState& state, FrameBuffer& framebuffer,
                               RenderStats& stats);

  std::vector<ClipVertex<single_precision>> transformed_;
};

} // namespace rtw::sw_renderer
