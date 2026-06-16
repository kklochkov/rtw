load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "core",
    srcs = ["obj_loader.cpp"],
    hdrs = [
        "camera.h",
        "clipping.h",
        "color.h",
        "color_buffer.h",
        "depth_buffer.h",
        "mesh.h",
        "obj_loader.h",
        "ostream.h",
        "precision.h",
        "projection.h",
        "raster_common.h",
        "render_stats.h",
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
        ":core",
        "//math:format",
        "@fmt",
    ],
)
