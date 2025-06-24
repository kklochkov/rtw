load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "sw_renderer_tests",
    size = "small",
    srcs = [
        "color_buffer_test.cpp",
        "obj_loader_test.cpp",
        "tex_coord_test.cpp",
    ],
    data = ["//sw_renderer/resources:cube"],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer",
        "//sw_renderer:format",
        "@googletest//:gtest_main",
    ],
)
