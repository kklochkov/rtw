# Software render

Simple 3D Software render that implements following features:
- Line drawing using the DDA (Digital Differential Analyzer) and the Bresenham's algorithms. See [sw_renderer](../../sw_renderer/rasterisation_routines.h).
- Flat shading using the barycentric coordinates. See [sw_renderer](../../sw_renderer/rasterisation_routines.h).
- Face culling by rejecting triangles based on their winding order. The render uses `right-handed` coordinate system and `counter clockwise` polygons `winding order`.
- Perspective correct texture mapping
- Possibility to load meshes in `Wavefront OBJ` format

## Build & run

To run the application, use the following command:
```bash
bazel run //sandbox/sw_renderer -c opt
```

It is possible to run custom mesh by supplying path to a mesh in `--mesh` argument. For example:
```bash
bazel run //sandbox/sw_renderer -c opt -- --mesh path/to/my/mesh.obj
```

Note the `-c opt` flag, which is used to run the application in release mode to get better performance.
It's possible to run the demo application that uses fixed-point arithmetic by adding the `--define=use_fixed_point`.
