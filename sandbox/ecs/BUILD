load("//:cc_defs.bzl", "cc_binary_with_llvm_symbolizer")

cc_binary_with_llvm_symbolizer(
    name = "ecs",
    srcs = [
        "application.cpp",
        "application.h",
        "events.h",
        "main.cpp",
    ],
    deps = [
        "//constants:render_constants",
        "//constants:time_constants",
        "//ecs",
        "//event_bus",
        "//stl",
        "//sw_renderer:core",
        "//third_party/imgui:sdl2_backend",
        "@cli11",
        "@fmt",
        "@sysroot//:SDL2",
        "@sysroot//:SDL2_image",
    ],
)
