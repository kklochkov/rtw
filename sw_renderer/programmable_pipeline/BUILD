load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "programmable_pipeline",
    srcs = ["pipeline.cpp"],
    hdrs = [
        "builtin_shaders.h",
        "clip_space.h",
        "frame_buffer.h",
        "pipeline.h",
        "pipeline_rasterisation.h",
        "pipeline_state.h",
        "register_file.h",
        "sampler.h",
        "shader.h",
        "shader_builtins.h",
        "varyings.h",
        "vertex_layout.h",
        "vertex_stream.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//math",
        "//stl",
        "//sw_renderer:core",
    ],
)
