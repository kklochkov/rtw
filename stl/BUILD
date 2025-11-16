load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "stl",
    hdrs = [
        "circular_buffer.h",
        "flags.h",
        "graph.h",
        "heap_array.h",
        "id.h",
        "iterator.h",
        "packed_buffer.h",
        "span.h",
        "static_contiguous_storage.h",
        "static_flat_unordered_map.h",
        "static_flat_unordered_set.h",
        "static_queue.h",
        "static_stack.h",
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
