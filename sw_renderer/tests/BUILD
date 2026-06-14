load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "sw_renderer_tests",
    size = "small",
    srcs = [
        "clipping_test.cpp",
        "color_buffer_test.cpp",
        "color_test.cpp",
        "depth_buffer_test.cpp",
        "frame_buffer_test.cpp",
        "obj_loader_test.cpp",
        "pipeline_state_test.cpp",
        "projection_test.cpp",
        "rasterisation_test.cpp",
        "register_file_test.cpp",
        "renderer_test.cpp",
        "sampler_test.cpp",
        "shader_builtins_test.cpp",
        "tex_coord_test.cpp",
        "varyings_test.cpp",
        "vertex_layout_test.cpp",
        "vertex_stream_test.cpp",
    ],
    data = ["//sw_renderer/resources:cube"],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer",
        "//sw_renderer:format",
        "@googletest//:gtest_main",
    ],
)

# Built with RTW_USE_FIXED_POINT so single_precision is FixedPoint16 and double_precision is
# FixedPoint32. Guards the fixed-point rasterisation path (edge functions widened to FixedPoint32 in
# fill_triangle_bbox) against the cross-product overflow that corrupts large triangles.
cc_test_with_llvm_symbolizer(
    name = "rasterisation_fixed_point_test",
    size = "small",
    srcs = ["rasterisation_fixed_point_test.cpp"],
    local_defines = ["RTW_USE_FIXED_POINT"],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)

# Compile-smoke for the Step 2 shader-support primitives (Color <-> Vector4 conversions, Sampler2D,
# shader built-ins) under RTW_USE_FIXED_POINT, so single_precision is FixedPoint16. Proves these
# header-only templates instantiate and run in fixed-point mode; the exhaustive exact assertions live
# in the float sw_renderer_tests target.
cc_test_with_llvm_symbolizer(
    name = "shader_builtins_fixed_point_test",
    size = "small",
    srcs = ["shader_builtins_fixed_point_test.cpp"],
    local_defines = ["RTW_USE_FIXED_POINT"],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)

# Compile-smoke for the Step 4 vertex-input primitives (vertex_layout / vertex_stream) under
# RTW_USE_FIXED_POINT, so single_precision is FixedPoint16. Proves the typed stream and the raw decode
# path instantiate in fixed-point mode; the exact assertions live in the float sw_renderer_tests target.
cc_test_with_llvm_symbolizer(
    name = "vertex_input_fixed_point_test",
    size = "small",
    srcs = ["vertex_input_fixed_point_test.cpp"],
    local_defines = ["RTW_USE_FIXED_POINT"],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)
