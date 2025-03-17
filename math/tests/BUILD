load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "math_tests",
    size = "small",
    srcs = [
        "angle_test.cpp",
        "barycentric_test.cpp",
        "convex_polygon_test.cpp",
        "frustum_test.cpp",
        "interpolation_test.cpp",
        "intersection_test.cpp",
        "matrix_test.cpp",
        "point_test.cpp",
        "transform2_test.cpp",
        "transform3_test.cpp",
        "vector_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//math",
        "//math:format",
        "@googletest//:gtest_main",
    ],
)
