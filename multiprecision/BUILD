load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "multiprecision",
    hdrs = [
        "fixed_point.h",
        "int128.h",
        "math.h",
        "operations.h",
        "rational.h",
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
