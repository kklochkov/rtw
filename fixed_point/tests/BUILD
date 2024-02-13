load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "fixed_point_test",
    srcs = [
        "fixed_point_test.cpp",
        "int128_test.cpp",
    ],
    deps = [
        "//fixed_point",
        "@googletest//:gtest_main",
    ],
)
