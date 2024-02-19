load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "rasterisation_becnhmark",
    srcs = ["rasterisation_benchmark.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//sw_renderer",
        "@google_benchmark//:benchmark_main",
    ],
)
