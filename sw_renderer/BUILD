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
        "flags.h",
        "format.h",
        "mesh.h",
        "obj_loader.h",
        "projection.h",
        "rasterisation_routines.h",
        "renderer.h",
        "tex_coord.h",
        "texture.h",
        "vertex.h",
    ],
    #    defines = ["DEBUG_DRAWING"],
    visibility = ["//visibility:public"],
    deps = [
        "//math",
        "//math:format",
        "@fmt",
    ],
)
