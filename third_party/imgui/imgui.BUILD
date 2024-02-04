load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui",
    srcs = [
        "backends/imgui_impl_opengl3.cpp",
        "backends/imgui_impl_opengl3_loader.h",
        "backends/imgui_impl_sdl2.cpp",
        "backends/imgui_impl_sdlrenderer.cpp",
        "imconfig.h",
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    ],
    hdrs = [
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_sdl2.h",
        "backends/imgui_impl_sdlrenderer.h",
        "imgui.h",
    ],
    defines = ["IMGUI_DEFINE_MATH_OPERATORS"],
    includes = [
        ".",
        "backends",
    ],
    linkopts = select({
        "@platforms//os:linux": ["-lGL"],
        "@platforms//os:osx": ["-framework OpenGL"],
    }),
    deps = select({
        "@platforms//os:linux": ["@sysroot_linux//:sdl2"],
        "@platforms//os:osx": ["@sysroot_macos//:sdl2"],
    }),
)
