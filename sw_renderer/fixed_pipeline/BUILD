load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fixed_pipeline",
    srcs = ["renderer.cpp"],
    hdrs = [
        "rasterisation_routines.h",
        "renderer.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//math",
        "//stl",
        "//sw_renderer:core",
    ],
)
