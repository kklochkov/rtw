load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "math_tests",
    srcs = [
        "angle_test.cpp",
        "convex_polygon_test.cpp",
        "interpolation_test.cpp",
        "matrix_test.cpp",
        "operations_test.cpp",
        "point_test.cpp",
        "transform2_test.cpp",
        "transform3_test.cpp",
        "vector_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//math",
        "@googletest//:gtest_main",
    ],
)
