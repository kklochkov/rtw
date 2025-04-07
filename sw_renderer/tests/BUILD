load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "stl_tests",
    size = "small",
    srcs = [
        "circular_buffer_test.cpp",
        "contiguous_storage_test.cpp",
        "packed_buffer_test.cpp",
        "queue_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer:stl",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_llvm_symbolizer(
    name = "sw_renderer_tests",
    size = "small",
    srcs = [
        "color_buffer_test.cpp",
        "flags_test.cpp",
        "obj_loader_test.cpp",
        "tex_coord_test.cpp",
    ],
    data = ["//sw_renderer/resources:cube"],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)
