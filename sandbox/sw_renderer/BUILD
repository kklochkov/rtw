load("@rules_cc//cc:defs.bzl", "cc_library")
load("//:cc_defs.bzl", "cc_binary_with_fixed_point", "cc_binary_with_llvm_symbolizer")

cc_library(
    name = "imgui_sdl2_backend",
    srcs = ["@imgui//:sdl2_backend"],
    tags = ["no-clang-tidy"],
    deps = [
        "@imgui",
        "@sysroot//:SDL2",
    ],
)

cc_binary_with_llvm_symbolizer(
    name = "sw_renderer",
    srcs = ["main.cpp"],
    data = ["//sw_renderer/resources:textured_cube"],
    deps = [
        ":imgui_sdl2_backend",
        "//constants:time_constants",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)

# Fixed-point build of the demo. The cc_binary_with_fixed_point transition
# recompiles the whole dependency graph (including //sw_renderer) with
# -DRTW_USE_FIXED_POINT, so single_precision becomes FixedPoint16 end-to-end.
# Tagged "manual" so it is excluded from //... wildcard builds; run it explicitly:
#   bazel run //sandbox/sw_renderer:fp -c opt
cc_binary_with_fixed_point(
    name = "fp",
    srcs = ["main.cpp"],
    data = ["//sw_renderer/resources:textured_cube"],
    tags = ["manual"],
    deps = [
        ":imgui_sdl2_backend",
        "//constants:time_constants",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)
