load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "sw_renderer",
    srcs = [
        "obj_loader.cpp",
        "renderer.cpp",
    ],
    hdrs = [
        "camera.h",
        "clipping.h",
        "color.h",
        "color_buffer.h",
        "depth_buffer.h",
        "mesh.h",
        "obj_loader.h",
        "precision.h",
        "projection.h",
        "rasterisation_routines.h",
        "renderer.h",
        "tex_coord.h",
        "texture.h",
        "types.h",
        "vertex.h",
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
