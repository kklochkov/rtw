load("//:cc_test_with_llvm_symbolizer.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "fixed_point_test",
    srcs = [
        "fixed_point_test.cpp",
        "int128_test.cpp",
        "operations_test.cpp",
    ],
    deps = [
        "//fixed_point",
        "@googletest//:gtest_main",
    ],
)
