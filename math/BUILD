load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "math",
    hdrs = [
        "angle.h",
        "barycentric.h",
        "convex_polygon.h",
        "frustum.h",
        "interpolation.h",
        "intersection.h",
        "matrix.h",
        "plane.h",
        "point.h",
        "transform2.h",
        "transform3.h",
        "vector.h",
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
