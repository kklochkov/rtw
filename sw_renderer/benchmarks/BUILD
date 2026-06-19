load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//:cc_defs.bzl", "cc_binary_with_fixed_point")

cc_binary(
    name = "rasterisation_becnhmark",
    srcs = ["rasterisation_benchmark.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//math",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "//sw_renderer/programmable_pipeline",
        "@google_benchmark//:benchmark_main",
    ],
)

cc_binary(
    name = "pipeline_benchmark",
    srcs = ["pipeline_benchmark.cpp"],
    tags = ["no-clang-tidy"],
    deps = [
        "//math",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "//sw_renderer/programmable_pipeline",
        "@google_benchmark//:benchmark_main",
    ],
)

# Fixed-point variant: the transition recompiles the whole dependency graph with
# -DRTW_USE_FIXED_POINT, so this characterises the per-fragment cost with
# single_precision = FixedPoint16. Tagged "manual" so the wildcard build/test of
# //... does not pull in the transitioned configuration.
cc_binary_with_fixed_point(
    name = "pipeline_benchmark_fp",
    srcs = ["pipeline_benchmark.cpp"],
    tags = [
        "manual",
        "no-clang-tidy",
    ],
    deps = [
        "//math",
        "//stl",
        "//sw_renderer:core",
        "//sw_renderer/fixed_pipeline",
        "//sw_renderer/programmable_pipeline",
        "@google_benchmark//:benchmark_main",
    ],
)
