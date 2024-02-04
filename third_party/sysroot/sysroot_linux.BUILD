load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sdl2",
    srcs = ["lib/aarch64-linux-gnu/libSDL2.so"],
    hdrs = glob(["include/SDL2/*"]),
    includes = ["include/SDL2"],
)

cc_library(
    name = "sdl2_image",
    srcs = ["lib/aarch64-linux-gnu/libSDL2_image.so"],
    hdrs = glob(["include/SDL2/*"]),
    includes = ["include/SDL2"],
)
