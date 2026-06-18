load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "fixed_point_benchmark",
    srcs = ["fixed_point_benchmark.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//multiprecision",
        "@google_benchmark//:benchmark_main",
    ],
)
