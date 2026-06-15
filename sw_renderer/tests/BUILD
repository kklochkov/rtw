load("//:cc_defs.bzl", "cc_test_with_fixed_point", "cc_test_with_llvm_symbolizer")

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
        "pipeline_test.cpp",
        "projection_test.cpp",
        "rasterisation_test.cpp",
        "register_file_test.cpp",
        "renderer_test.cpp",
        "sampler_test.cpp",
        "shader_builtins_test.cpp",
        "shader_test.cpp",
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

cc_test_with_fixed_point(
    name = "sw_renderer_fixed_point_tests",
    size = "small",
    srcs = [
        "clipping_fixed_point_test.cpp",
        "pipeline_fixed_point_test.cpp",
        "rasterisation_fixed_point_test.cpp",
        "shader_builtins_fixed_point_test.cpp",
        "shader_fixed_point_test.cpp",
        "vertex_input_fixed_point_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)
