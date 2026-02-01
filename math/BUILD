load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "math",
    hdrs = [
        "angle.h",
        "barycentric.h",
        "barycentric_operations.h",
        "convex_polygon.h",
        "convex_polygon_operations.h",
        "frustum.h",
        "interpolation.h",
        "intersection.h",
        "matrix.h",
        "matrix_decomposition.h",
        "matrix_eigen_decomposition.h",
        "matrix_operations.h",
        "plane.h",
        "point.h",
        "transform2.h",
        "transform3.h",
        "vector.h",
        "vector_operations.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//constants:math_constants",
        "//multiprecision",
    ],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":math",
        "//multiprecision:format",
        "@fmt",
    ],
)
