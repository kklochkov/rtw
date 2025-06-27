load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "ecs_tests",
    size = "small",
    srcs = ["ecs_test.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//ecs",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)
