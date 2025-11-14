load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "stl",
    hdrs = [
        "circular_buffer.h",
        "contiguous_storage.h",
        "flags.h",
        "flat_unordered_map.h",
        "flat_unordered_set.h",
        "graph.h",
        "heap_array.h",
        "id.h",
        "iterator.h",
        "packed_buffer.h",
        "queue.h",
        "span.h",
        "stack.h",
        "static_string.h",
        "static_vector.h",
        "string_view.h",
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
