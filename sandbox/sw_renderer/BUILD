load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "sw_renderer",
    srcs = [
        "main.cpp",
        "@imgui//:sdl2_backend",
    ],
    data = ["//sw_renderer/resources:textured_cube"],
    deps = [
        "//sw_renderer",
        "@cli11",
        "@fmt",
        "@imgui",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)
