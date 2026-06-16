load("//:cc_defs.bzl", "cc_test_with_fixed_point", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "programmable_pipeline_tests",
    size = "small",
    srcs = [
        "builtin_shaders_test.cpp",
        "clip_space_test.cpp",
        "frame_buffer_test.cpp",
        "pipeline_rasterisation_test.cpp",
        "pipeline_state_test.cpp",
        "pipeline_test.cpp",
        "register_file_test.cpp",
        "sampler_test.cpp",
        "shader_builtins_test.cpp",
        "shader_test.cpp",
        "varyings_test.cpp",
        "vertex_layout_test.cpp",
        "vertex_stream_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer:core",
        "//sw_renderer/programmable_pipeline",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_fixed_point(
    name = "programmable_pipeline_fixed_point_tests",
    size = "small",
    srcs = [
        "builtin_shaders_fixed_point_test.cpp",
        "clipping_fixed_point_test.cpp",
        "pipeline_fixed_point_test.cpp",
        "pipeline_rasterisation_fixed_point_test.cpp",
        "shader_builtins_fixed_point_test.cpp",
        "shader_fixed_point_test.cpp",
        "vertex_input_fixed_point_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer:core",
        "//sw_renderer/programmable_pipeline",
        "@googletest//:gtest_main",
    ],
)
