load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "stl",
    hdrs = [
        "circular_buffer.h",
        "contiguous_storage.h",
        "flags.h",
        "packed_buffer.h",
        "queue.h",
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
