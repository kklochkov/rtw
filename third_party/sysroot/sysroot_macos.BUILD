load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sdl2",
    srcs = ["lib/libSDL2.dylib"],
    hdrs = glob(["include/SDL2/*"]),
    includes = ["include/SDL2"],
)

cc_library(
    name = "sdl2_image",
    srcs = ["lib/libSDL2_image.dylib"],
    hdrs = glob(["include/SDL2/*"]),
    includes = ["include/SDL2"],
)
