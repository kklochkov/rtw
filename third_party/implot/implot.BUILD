load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "implot",
    srcs = [
        "implot.cpp",
        "implot_demo.cpp",
        "implot_internal.h",
        "implot_items.cpp",
    ],
    hdrs = ["implot.h"],
    includes = ["."],
    deps = ["@imgui"],
)
