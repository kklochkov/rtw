#pragma once

#include "sw_renderer/clip_space.h"
#include "sw_renderer/clipping.h"
#include "sw_renderer/color.h"
#include "sw_renderer/color_buffer.h"
#include "sw_renderer/depth_buffer.h"
#include "sw_renderer/frame_buffer.h"
#include "sw_renderer/pipeline_state.h"
#include "sw_renderer/precision.h"
#include "sw_renderer/rasterisation_routines.h"
#include "sw_renderer/register_file.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/shader.h"
#include "sw_renderer/types.h"
#include "sw_renderer/vertex_stream.h"

#include "math/vector.h"
#include "math/vector_operations.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace rtw::sw_renderer
{

/// Stateless OpenGL-style triangle pipeline driving a programmable `IShaderProgram`.
///
/// The pipeline owns only reusable scratch (the post-vertex-shader vertices);
/// every per-draw input -- shader program, vertex stream, optional index buffer,
/// `PipelineState` and `FrameBuffer` -- is passed by reference, so a higher-level renderer can own the
/// framebuffer/state and drive a shared `Pipeline`.
///
/// Each `draw_*` runs the full:
///  1. vertex shader ->
///  2. clip-space clip ->
///  3. perspective divide ->
///  4. viewport / depth-range ->
///  5. cull ->
///  6. rasterise ->
///  7. scissor ->
///  8. depth test ->
///  9. fragment shader ->
/// 10. depth write ->
/// 11. colour write
class Pipeline
{
public:
  /// Renders triangles assembled from consecutive vertex triples (`TRIANGLES`, non-indexed).
  void draw_arrays(const IShaderProgram& program, const RawVertexStream& vertices, const PipelineState& state,
                   FrameBuffer& framebuffer, RenderStats& stats);

  /// Renders triangles assembled from consecutive index triples into the vertex stream (`TRIANGLES`).
  void draw_elements(const IShaderProgram& program, const RawVertexStream& vertices, const IndexBuffer& indices,
                     const PipelineState& state, FrameBuffer& framebuffer, RenderStats& stats);

private:
  /// Runs the vertex shader over every vertex once into `transformed_` (so indexed draws reuse shared vertices),
  /// yielding clip-space position + varyings per vertex.
  void transform_vertices(const IShaderProgram& program, const RawVertexStream& vertices);

  /// Processes one assembled (clip-space) triangle:
  /// clip -> triangulate -> per sub-triangle divide / viewport / cull / rasterise, accumulating `stats`.
  static void process_triangle(const IShaderProgram& program, const ClipVertex<single_precision>& v0,
                               const ClipVertex<single_precision>& v1, const ClipVertex<single_precision>& v2,
                               std::uint32_t primitive_id, const PipelineState& state, FrameBuffer& framebuffer,
                               RenderStats& stats);

  std::vector<ClipVertex<single_precision>> transformed_; ///< Post-vertex-shader scratch, reused across draws.
};

} // namespace rtw::sw_renderer
