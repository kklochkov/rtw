load("@rules_cc//cc:defs.bzl", "cc_library")
load("//:cc_defs.bzl", "cc_binary_with_llvm_symbolizer")

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
        "//sw_renderer",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)
