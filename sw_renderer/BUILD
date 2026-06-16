load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "sw_renderer",
    srcs = [
        "obj_loader.cpp",
        "pipeline.cpp",
        "renderer.cpp",
    ],
    hdrs = [
        "builtin_shaders.h",
        "camera.h",
        "clip_space.h",
        "clipping.h",
        "color.h",
        "color_buffer.h",
        "depth_buffer.h",
        "frame_buffer.h",
        "mesh.h",
        "obj_loader.h",
        "ostream.h",
        "pipeline.h",
        "pipeline_state.h",
        "precision.h",
        "projection.h",
        "rasterisation_routines.h",
        "register_file.h",
        "render_stats.h",
        "renderer.h",
        "sampler.h",
        "shader.h",
        "shader_builtins.h",
        "tex_coord.h",
        "texture.h",
        "types.h",
        "varyings.h",
        "vertex.h",
        "vertex_layout.h",
        "vertex_stream.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//math",
        "//stl",
    ],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":sw_renderer",
        "//math:format",
        "@fmt",
    ],
)
