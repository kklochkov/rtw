load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "event_bus_tests",
    size = "small",
    srcs = ["event_bus_test.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//event_bus",
        "@googletest//:gtest_main",
    ],
)
