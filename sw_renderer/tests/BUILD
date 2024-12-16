load("//:cc_test_with_llvm_symbolizer.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "sw_renderer_tests",
    srcs = [
        "barycentric_test.cpp",
        "color_buffer_test.cpp",
        "flags_test.cpp",
        "obj_loader_test.cpp",
        "operations_test.cpp",
        "projection_test.cpp",
        "tex_coord_test.cpp",
    ],
    data = ["//sw_renderer/resources:cube"],
    deps = [
        "//sw_renderer",
        "@googletest//:gtest_main",
    ],
)
