load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "math",
    hdrs = [
        "angle.h",
        "convex_polygon.h",
        "interpolation.h",
        "matrix.h",
        "operations.h",
        "point.h",
        "transform2.h",
        "transform3.h",
        "vector.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//constants:math_constants",
        "//fixed_point",
    ],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":math",
        "//fixed_point:format",
        "@fmt",
    ],
)
