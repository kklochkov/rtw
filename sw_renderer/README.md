# sw_renderer

Software triangle rasterizer with configurable precision (IEEE 754 float or fixed-point via `RTW_USE_FIXED_POINT`).

## Features

- **Triangle rasterization** — bounding-box and scanline fill algorithms with depth buffering
- **Sutherland-Hodgman clipping** — triangles clipped against a 6-plane frustum
- **Perspective-correct interpolation** — texture coordinates and vertex attributes
- **Wavefront OBJ/MTL loading** — meshes, materials, and texture maps
- **Configurable render modes** — wireframe, shading, texturing, lighting, face culling, normals (bitfield flags)
- **Fixed-point support** — all core math uses `single_precision` type alias (float or FixedPoint16)

## File Overview

| File | Description |
|------|-------------|
| `precision.h` | Scalar type aliases (`single_precision`, `double_precision`, `ULP`) |
| `types.h` | Type aliases for vectors, points, matrices, angles, barycentric coords |
| `color.h` | Packed RGBA color (4 bytes) with saturating arithmetic |
| `vertex.h` | Vertex struct (position, tex coord, normal, color) |
| `tex_coord.h` | Texture coordinate (u, v) wrapper |
| `texture.h` | Texture image (pixel data + dimensions) |
| `mesh.h` | Mesh struct (vertices, faces, materials, textures) |
| `obj_loader.h` | Wavefront .obj / .mtl file parsing |
| `projection.h` | Screen-space and NDC transformation matrices |
| `clipping.h` | Sutherland-Hodgman polygon clipping + triangulation |
| `rasterisation_routines.h` | Low-level rasterization (fill, interpolation, depth test) |
| `renderer.h` | High-level Renderer class with render mode flags and stats |
| `renderer.cpp` | Renderer implementation (mesh drawing pipeline) |
| `camera.h` | Camera class (view matrix, movement) |
| `color_buffer.h` | Framebuffer for pixel data |
| `depth_buffer.h` | Z-buffer |
| `ostream.h` | Stream formatting for Color and TexCoord |
| `format.h` | fmt/std::format integration |

## Architecture

```
OBJ/MTL file
    |
    v
 load_obj() --> Mesh (vertices, faces, materials, textures)
    |
    v
 Renderer::draw_mesh()
    |-- transform_face_vertices()    (model -> world space)
    |-- setup_normals()              (per-vertex or face normal)
    |-- calculate_light_intensity()  (dot product with light dir)
    |-- Face culling                 (back-face removal)
    |-- clip()                       (Sutherland-Hodgman against frustum)
    |-- triangulate()                (polygon -> triangle fan)
    |-- project_to_screen()          (clip -> NDC -> screen space)
    |-- fill_triangle_bbox()         (rasterize with depth test)
    v
 ColorBuffer + DepthBuffer --> pixel data
```

## Rendering Pipeline Stats

The `RenderStats` struct tracks per-frame:
- `triangles_submitted` — total triangles before any culling
- `triangles_clipped` — triangles fully outside the frustum
- `triangles_culled` — triangles removed by back-face culling
- `triangles_rendered` — triangles that produced pixels

## Build & Test

```bash
bazel test //sw_renderer/tests:sw_renderer_tests
bazel run //sandbox/sw_renderer:sw_renderer  # interactive demo
```

## Design Notes

- Color uses saturating addition (`operator+`) and clamped `operator*` to prevent overflow/UB
- Float-to-byte Color constructor clamps inputs to [0.0, 1.0] before conversion
- Texture coordinate sampling clamps to `[0, width-1]` / `[0, height-1]` to prevent one-past-end access
- Template clipping code uses `T{0}` literals (not `0.0F`) for FixedPoint compatibility
- Mesh lookup methods (`material()`, `texture()`) use `find()` + `assert` pattern (check `has_*()` first)
- `#include <cassert>` is explicit in `mesh.h` for header self-containedness
