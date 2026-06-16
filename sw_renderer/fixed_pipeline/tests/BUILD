load("//:cc_defs.bzl", "cc_test_with_fixed_point", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "fixed_pipeline_tests",
    size = "small",
    srcs = [
        "rasterisation_test.cpp",
        "renderer_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_fixed_point(
    name = "fixed_pipeline_fixed_point_tests",
    size = "small",
    srcs = ["rasterisation_fixed_point_test.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "@googletest//:gtest_main",
    ],
)
