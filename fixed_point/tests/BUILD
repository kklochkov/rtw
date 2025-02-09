load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "fixed_point_test",
    size = "small",
    srcs = [
        "fixed_point_test.cpp",
        "int128_test.cpp",
        "math_tests.cpp",
        "operations_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//fixed_point",
        "@googletest//:gtest_main",
    ],
)
