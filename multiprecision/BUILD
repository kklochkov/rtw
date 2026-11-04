load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

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
    deps = [
        "//constants:math_constants",
        "//stl:utility",
    ],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    deps = [
        ":multiprecision",
        "@fmt",
    ],
)
