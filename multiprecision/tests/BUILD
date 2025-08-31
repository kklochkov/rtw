load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "multiprecision_test",
    size = "small",
    srcs = [
        "fixed_point_test.cpp",
        "int128_test.cpp",
        "math_tests.cpp",
        "operations_test.cpp",
        "rational_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "//multiprecision:format",
        "@googletest//:gtest_main",
    ],
)
