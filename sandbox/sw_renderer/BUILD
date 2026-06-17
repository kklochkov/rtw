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

cc_binary_with_llvm_symbolizer(
    name = "programmable",
    srcs = ["main_programmable.cpp"],
    data = ["//sw_renderer/resources:textured_cube"],
    deps = [
        ":imgui_sdl2_backend",
        "//constants:time_constants",
        "//math",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/programmable_pipeline",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)

cc_binary_with_fixed_point(
    name = "programmable_fp",
    srcs = ["main_programmable.cpp"],
    data = ["//sw_renderer/resources:textured_cube"],
    tags = ["manual"],
    deps = [
        ":imgui_sdl2_backend",
        "//constants:time_constants",
        "//math",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/programmable_pipeline",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)
