load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "multiprecision",
    hdrs = [
        "fixed_point.h",
        "fixed_point_math.h",
        "int128.h",
        "int128_math.h",
        "math.h",
        "ostream.h",
        "rational.h",
        "rational_math.h",
    ],
    visibility = ["//visibility:public"],
    deps = ["//constants:math_constants"],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":multiprecision",
        "@fmt",
    ],
)
