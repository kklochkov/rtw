load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "stl",
    hdrs = [
        "circular_buffer.h",
        "contiguous_storage.h",
        "flags.h",
        "flat_unordered_map.h",
        "heap_array.h",
        "packed_buffer.h",
        "queue.h",
        "static_vector.h",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":stl",
        "@fmt",
    ],
)
