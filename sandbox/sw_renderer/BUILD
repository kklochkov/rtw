load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "sw_renderer",
    srcs = [
        "main.cpp",
        "@imgui//:sdl2_backend",
    ],
    data = ["//sw_renderer/resources:textured_cube"],
    linkopts = select({
        "@platforms//os:linux": ["-lGL"],
        "@platforms//os:osx": ["-framework OpenGL"],
    }),
    deps = [
        "//sw_renderer",
        "@cli11",
        "@fmt",
        "@imgui",
    ] + select({
        "@platforms//os:linux": [
            "@sysroot_linux//:sdl2",
            "@sysroot_linux//:sdl2_image",
        ],
        "@platforms//os:osx": [
            "@sysroot_macos//:sdl2",
            "@sysroot_macos//:sdl2_image",
        ],
    }),
)
